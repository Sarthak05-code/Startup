
#include <ctype.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DOS_HEADER_SIZE 64
#define COFF_HEADER_SIZE 20
#define SECTION_HEADER_SIZE 40
#define COFF_SYMBOL_SIZE 18

// Read a 16-bit little-endian value.
uint16_t read_u16_le(const unsigned char *data) {
  return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

// Read a 32-bit little-endian value.
uint32_t read_u32_le(const unsigned char *data) {
  return (uint32_t)data[0] | ((uint32_t)data[1] << 8) |
         ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
}

// Resolve a section name.
// Returns a pointer to the name or NULL if invalid.
const char *get_section_name(const unsigned char *raw_name,
                             const unsigned char *string_table,
                             uint32_t string_table_size, char direct_name[9]) {
  memcpy(direct_name, raw_name, 8);
  direct_name[8] = '\0';

  // Normal section name.
  if (raw_name[0] != '/') {
    return direct_name;
  }

  // Long section name requires a String Table.
  if (string_table == NULL) {
    return NULL;
  }

  // Extract the decimal offset after '/'.
  uint32_t name_offset = 0;
  int digit_found = 0;
  int null_found = 0;

  for (int i = 1; i < 8; i++) {

    unsigned char ch = raw_name[i];

    if (ch == '\0') {
      null_found = 1;
      continue;
    }

    // Characters after the null terminator must be zero.
    if (null_found) {
      return NULL;
    }

    if (ch < '0' || ch > '9') {
      return NULL;
    }

    digit_found = 1;

    name_offset = name_offset * 10 + (ch - '0');
  }

  if (!digit_found) {
    return NULL;
  }

  // Valid offsets must point inside the String Table,
  // after its four-byte size field.
  if (name_offset < 4 || name_offset >= string_table_size) {
    return NULL;
  }

  const char *name_location = (const char *)(string_table + name_offset);

  size_t remaining_bytes = string_table_size - name_offset;

  // Verify that the name ends within the String Table.
  if (memchr(name_location, '\0', remaining_bytes) == NULL) {
    return NULL;
  }

  return name_location;
}

int main(void) {

  FILE *source = NULL;
  unsigned char *string_table = NULL;
  int result = 1;

  char filename[256];

  printf("Enter exe filename: ");

  if (scanf("%255s", filename) != 1) {
    printf("Invalid filename input.\n");
    goto cleanup;
  }

  source = fopen(filename, "rb");

  if (source == NULL) {
    printf("File not able to open.\n");
    goto cleanup;
  }

  printf("File opened successfully.\n");

  // --------------------------------------------------
  // STEP 1: Determine file size
  // --------------------------------------------------

  if (fseek(source, 0, SEEK_END) != 0) {
    printf("Unable to determine file size.\n");
    goto cleanup;
  }

  long file_size = ftell(source);

  if (file_size < 0) {
    printf("Unable to determine file size.\n");
    goto cleanup;
  }

  if (file_size < DOS_HEADER_SIZE) {
    printf("File is too small to contain a DOS Header.\n");
    goto cleanup;
  }

  // --------------------------------------------------
  // STEP 2: Read DOS Header
  // --------------------------------------------------

  if (fseek(source, 0, SEEK_SET) != 0) {
    printf("Unable to locate DOS Header.\n");
    goto cleanup;
  }

  unsigned char dos_header[DOS_HEADER_SIZE];

  size_t bytes_read = fread(dos_header, 1, DOS_HEADER_SIZE, source);

  if (bytes_read != DOS_HEADER_SIZE) {
    printf("Could not read complete DOS Header.\n");
    goto cleanup;
  }

  if (dos_header[0] != 'M' || dos_header[1] != 'Z') {
    printf("Invalid DOS signature.\n");
    goto cleanup;
  }

  printf("DOS signature: MZ OK.\n");

  uint32_t pe_offset = read_u32_le(&dos_header[0x3C]);

  printf("PE header offset: 0x%08" PRIX32 "\n", pe_offset);

  // --------------------------------------------------
  // STEP 3: Locate and validate PE Signature
  // --------------------------------------------------

  if ((uint64_t)pe_offset + 4 > (uint64_t)file_size) {
    printf("PE Header offset is outside the file.\n");
    goto cleanup;
  }

  if (fseek(source, (long)pe_offset, SEEK_SET) != 0) {
    printf("Could not locate PE Header.\n");
    goto cleanup;
  }

  unsigned char pe_signature[4];

  bytes_read = fread(pe_signature, 1, 4, source);

  if (bytes_read != 4) {
    printf("Could not read PE Signature.\n");
    goto cleanup;
  }

  if (memcmp(pe_signature, "PE\0\0", 4) != 0) {
    printf("Invalid PE Signature.\n");
    goto cleanup;
  }

  printf("PE Signature: PE\\0\\0 OK.\n");

  // --------------------------------------------------
  // STEP 4: Read COFF Header
  // --------------------------------------------------

  unsigned char coff_header[COFF_HEADER_SIZE];

  bytes_read = fread(coff_header, 1, COFF_HEADER_SIZE, source);

  if (bytes_read != COFF_HEADER_SIZE) {
    printf("Could not read complete COFF Header.\n");
    goto cleanup;
  }

  printf("COFF Header read successfully.\n");

  uint16_t number_of_sections = read_u16_le(&coff_header[2]);

  uint32_t pointer_to_symbol_table = read_u32_le(&coff_header[8]);

  uint32_t number_of_symbols = read_u32_le(&coff_header[12]);

  uint16_t size_of_optional_header = read_u16_le(&coff_header[16]);

  printf("Pointer to Symbol Table: 0x%08" PRIX32 "\n", pointer_to_symbol_table);

  printf("Number of Symbols: %" PRIu32 "\n", number_of_symbols);

  printf("Number of Sections: %" PRIu16 "\n", number_of_sections);

  printf("Size of Optional Header: %" PRIu16 " bytes\n",
         size_of_optional_header);

  // --------------------------------------------------
  // STEP 5: Locate and load COFF String Table
  // --------------------------------------------------

  // Both values zero means the symbol table is absent.
  if (pointer_to_symbol_table == 0 && number_of_symbols == 0) {

    printf("COFF Symbol Table is not present.\n");

  } else {

    if (pointer_to_symbol_table == 0 || number_of_symbols == 0) {

      printf("Invalid COFF Symbol Table information.\n");
      goto cleanup;
    }

    // Use 64-bit arithmetic to prevent overflow.
    uint64_t symbol_table_size = (uint64_t)number_of_symbols * COFF_SYMBOL_SIZE;

    uint64_t string_table_offset =
        (uint64_t)pointer_to_symbol_table + symbol_table_size;

    // Verify that the symbol table is inside the file.
    if ((uint64_t)pointer_to_symbol_table > (uint64_t)file_size ||
        symbol_table_size > (uint64_t)file_size - pointer_to_symbol_table) {

      printf("COFF Symbol Table extends beyond the file.\n");
      goto cleanup;
    }

    printf("String Table Offset: 0x%08" PRIX64 "\n", string_table_offset);

    // At least four bytes must remain for the table size.
    if (string_table_offset > (uint64_t)file_size ||
        (uint64_t)file_size - string_table_offset < 4) {

      printf("String Table is outside the file.\n");
      goto cleanup;
    }

    // Read the first four bytes containing table size.
    unsigned char size_bytes[4];

    if (fseek(source, (long)string_table_offset, SEEK_SET) != 0) {
      printf("Could not locate String Table.\n");
      goto cleanup;
    }

    bytes_read = fread(size_bytes, 1, 4, source);

    if (bytes_read != 4) {
      printf("Could not read String Table size.\n");
      goto cleanup;
    }

    uint32_t string_table_size = read_u32_le(size_bytes);

    printf("String Table Size: %" PRIu32 " bytes\n", string_table_size);

    // Validate the table size against the actual file size.
    if (string_table_size < 4 ||
        (uint64_t)string_table_size >
            (uint64_t)file_size - string_table_offset) {

      printf("Invalid String Table size.\n");
      goto cleanup;
    }

    string_table = malloc(string_table_size);

    if (string_table == NULL) {
      printf("Memory allocation failed.\n");
      goto cleanup;
    }

    if (fseek(source, (long)string_table_offset, SEEK_SET) != 0) {
      printf("Could not locate String Table.\n");
      goto cleanup;
    }

    bytes_read = fread(string_table, 1, string_table_size, source);

    if (bytes_read != string_table_size) {
      printf("Could not read complete String Table.\n");
      goto cleanup;
    }

    printf("String Table loaded successfully.\n");
  }

  // --------------------------------------------------
  // STEP 6: Locate Section Table
  // --------------------------------------------------

  uint64_t section_table_offset =
      (uint64_t)pe_offset + 4 + COFF_HEADER_SIZE + size_of_optional_header;

  uint64_t section_table_size =
      (uint64_t)number_of_sections * SECTION_HEADER_SIZE;

  if (section_table_offset > (uint64_t)file_size ||
      section_table_size > (uint64_t)file_size - section_table_offset) {

    printf("Section Table extends beyond the file.\n");
    goto cleanup;
  }

  printf("Section Table Offset: 0x%08" PRIX64 "\n", section_table_offset);

  if (fseek(source, (long)section_table_offset, SEEK_SET) != 0) {
    printf("Could not locate Section Table.\n");
    goto cleanup;
  }

  printf("Section Table located successfully.\n");

  // --------------------------------------------------
  // STEP 7: Read and display Section Headers
  // --------------------------------------------------

  unsigned char section_header[SECTION_HEADER_SIZE];

  for (uint16_t i = 0; i < number_of_sections; i++) {

    bytes_read = fread(section_header, 1, SECTION_HEADER_SIZE, source);

    if (bytes_read != SECTION_HEADER_SIZE) {
      printf("Unable to read Section Header.\n");
      goto cleanup;
    }

    printf("\nSection Number: %" PRIu16 "\n", i + 1);

    // Resolve short and long section names.
    char direct_name[9];

    const char *section_name = get_section_name(
        section_header, string_table,
        string_table == NULL ? 0 : read_u32_le(string_table), direct_name);

    if (section_name == NULL) {
      printf("Invalid or unresolved section name.\n");
      goto cleanup;
    }

    printf("Section Name: %s\n", section_name);

    // Virtual Size
    uint32_t virtual_size = read_u32_le(&section_header[8]);

    printf("Virtual Size: 0x%08" PRIX32 "\n", virtual_size);

    // Virtual Address
    uint32_t virtual_address = read_u32_le(&section_header[12]);

    printf("Virtual Address: 0x%08" PRIX32 "\n", virtual_address);

    // Size of Raw Data
    uint32_t size_of_raw_data = read_u32_le(&section_header[16]);

    printf("Size of Raw Data: 0x%08" PRIX32 "\n", size_of_raw_data);

    // Pointer to Raw Data
    uint32_t pointer_to_raw_data = read_u32_le(&section_header[20]);

    printf("Pointer to Raw Data: 0x%08" PRIX32 "\n", pointer_to_raw_data);

    // Pointer to Relocations
    uint32_t pointer_to_relocations = read_u32_le(&section_header[24]);

    printf("Pointer to Relocations: 0x%08" PRIX32 "\n", pointer_to_relocations);

    // Pointer to Line Numbers
    uint32_t pointer_to_line_number = read_u32_le(&section_header[28]);

    printf("Pointer to Line Numbers: 0x%08" PRIX32 "\n",
           pointer_to_line_number);

    // Number of Relocations
    uint16_t number_of_relocations = read_u16_le(&section_header[32]);

    printf("Number of Relocations: %" PRIu16 "\n", number_of_relocations);

    // Number of Line Numbers
    uint16_t number_of_line_numbers = read_u16_le(&section_header[34]);

    printf("Number of Line Numbers: %" PRIu16 "\n", number_of_line_numbers);

    // Characteristics
    uint32_t characteristics = read_u32_le(&section_header[36]);

    printf("Characteristics: 0x%08" PRIX32 "\n", characteristics);

    if (characteristics & 0x20000000) {
      printf("Executable Section.\n");
    }

    if (characteristics & 0x40000000) {
      printf("Readable Section.\n");
    }

    if (characteristics & 0x00000020) {
      printf("Contains Code.\n");
    }

    if (characteristics & 0x80000000) {
      printf("Writable Section.\n");
    }

    if (characteristics & 0x00000040) {
      printf("Contains Initialized Data.\n");
    }

    if (characteristics & 0x00000080) {
      printf("Contains Uninitialized Data.\n");
    }

    if (characteristics & 0x02000000) {
      printf("Discardable Section.\n");
    }

    if (characteristics & 0x10000000) {
      printf("Shared Section.\n");
    }
  }

  printf("\nSection analysis completed successfully.\n");

  result = 0;

cleanup:

  // Release allocated memory and close the file.
  free(string_table);

  if (source != NULL) {
    fclose(source);
  }

  return result;
}
