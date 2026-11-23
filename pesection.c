#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DOS_HEADER_SIZE 64 // Size in bytes.
#define COFF_HEADER_SIZE 20
#define SECTION_HEADER_SIZE 40

int main() {
  unsigned char dos_header[DOS_HEADER_SIZE];
  char filename[256];
  printf("Enter exe filename: ");
  scanf("%255s", filename);
  FILE *source = fopen(filename, "rb");

  if (source == NULL) {
    printf("File not able to open.\n");
    return 1;
  }

  printf("File opened sucsessfully.\n");
  size_t bytes_read = fread(dos_header, 1, DOS_HEADER_SIZE, source);

  if (bytes_read != DOS_HEADER_SIZE) {
    printf("Error : Could not read complete DOS header.\n");
    fclose(source);
    return 1;
  }

  if (dos_header[0] != 'M' || dos_header[1] != 'Z') {
    printf("Error : Invalid DOS signature.\n");
    fclose(source);
    return 1;
  }
  printf("DOS signature := MZ OK.\n");

  uint32_t pe_offset =
      (uint32_t)dos_header[0x3C] | ((uint32_t)dos_header[0x3D] << 8) |
      ((uint32_t)dos_header[0x3E] << 16) | ((uint32_t)dos_header[0x3F] << 24);

  printf("PE header offset := 0x%08X\n", pe_offset);

  if (fseek(source, pe_offset, SEEK_SET) != 0) {
    printf("Error : Could not locate PE Header.\n");
    fclose(source);
    return 1;
  }

  unsigned char pe_signature[4];

  bytes_read = fread(pe_signature, 1, 4, source);

  if (bytes_read != 4) {
    printf("Error : Could not read PE Signature.\n");
    fclose(source);
    return 1;
  }

  if (memcmp(pe_signature, "PE\0\0", 4) != 0) {
    printf("Error : Invalid PE Signature.\n");
    fclose(source);
    return 1;
  }
  printf("PE Signature := PE\\0\\0 OK\n");

  unsigned char coff_header[COFF_HEADER_SIZE];

  bytes_read = fread(coff_header, 1, COFF_HEADER_SIZE, source);

  if (bytes_read != COFF_HEADER_SIZE) {
    printf("Error : Could not read complete COFF header.\n");
    fclose(source);
    return 1;
  }

  printf("COFF header : read sucessfully.\n");

  uint16_t number_of_sections =
      (uint16_t)coff_header[2] | ((uint16_t)coff_header[3] << 8);

  printf("Number of section := %u\n", number_of_sections);

  uint16_t size_of_optional_header =
      (uint16_t)coff_header[16] | ((uint16_t)coff_header[17] << 8);

  printf("Size of OptionalHeader := %u bytes\n", size_of_optional_header);

  long section_table_offset =
      (long)pe_offset + 4 + COFF_HEADER_SIZE + size_of_optional_header;
  printf("Section Table Offset := 0x%lX\n", section_table_offset);

  if (fseek(source, section_table_offset, SEEK_SET) != 0) {
    printf("Error : Could not locate the Section Table.\n");
    fclose(source);
    return 1;
  }
  printf("Sucessfully Located Section Table.\n");

  unsigned char section_header[SECTION_HEADER_SIZE];

  bytes_read = fread(section_header, 1, SECTION_HEADER_SIZE, source);

  if (bytes_read != SECTION_HEADER_SIZE) {
    printf("Error : Could not read Section Header.\n");
    fclose(source);
    return 1;
  }

  printf("Sucessfully Read : First Section Header.\n");

  char section_name[9];

  memcpy(section_name, section_header, 8);

  section_name[8] = '\0';

  printf("Section Name := %s\n", section_name);

  uint32_t virtual_size = (uint32_t)section_header[8] |
                          ((uint32_t)section_header[9] << 8) |
                          ((uint32_t)section_header[10] << 16) |
                          ((uint32_t)section_header[11] << 24);

  printf("Virtual Size : 0x%08X.\n", virtual_size);

  uint32_t virtual_address = (uint32_t)section_header[12] |
                             ((uint32_t)section_header[13] << 8) |
                             ((uint32_t)section_header[14] << 16) |
                             ((uint32_t)section_header[15] << 24);

  printf("Virtual Address : 0x%08X.\n", virtual_address);

  uint32_t size_of_raw_data = (uint32_t)section_header[16] |
                              ((uint32_t)section_header[17] << 8) |
                              ((uint32_t)section_header[18] << 16) |
                              ((uint32_t)section_header[19] << 24);

  printf("Size of raw bytes : 0x%08X.\n", size_of_raw_data);

  uint32_t pointer_to_raw_data = (uint32_t)section_header[20] |
                                 ((uint32_t)section_header[21] << 8) |
                                 ((uint32_t)section_header[22] << 16) |
                                 ((uint32_t)section_header[23] << 24);

  printf("Pointer to raw bytes : 0x%08X.\n", pointer_to_raw_data);

  fclose(source);
  return 0;
}
