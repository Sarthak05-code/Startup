#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DOS_HEADER_SIZE 64 // number represents bit
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

  fclose(source);
  return 0;
}
