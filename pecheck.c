/*
 * pecheck.c - PE explorer, steps 1 and 2
 *
 *   1. Check the "MZ" signature (file start) and the "PE\0\0" signature
 *   2. Read e_lfanew (the 4 bytes at 0x3C): the file offset of the PE header
 *
 * Usage:  pecheck [file]        (prompts for a file name if omitted)
 * Exit code: 0 = valid PE, 1 = not a PE file, 2 = could not open the file
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define DOS_HEADER_SIZE                                                        \
  64 /* e_lfanew is the last field of the 64-byte DOS header */
#define E_LFANEW_OFFSET 0x3C

/* Build a 32-bit number from 4 bytes stored lowest byte first (little-endian).
 * Doing it by hand works on any machine, whatever its own byte order. */
static uint32_t le32(const unsigned char *p) {
  return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
         ((uint32_t)p[3] << 24);
}

int main(int argc, char **argv) {
  char prompt_name[512];
  const char *path;

  if (argc > 1) {
    path = argv[1];
  } else {
    printf("Enter filename: ");
    if (fgets(prompt_name, sizeof(prompt_name), stdin) == NULL) {
      fprintf(stderr, "Input error.\n");
      return 2;
    }
    prompt_name[strcspn(prompt_name, "\r\n")] = '\0';
    path = prompt_name;
  }

  FILE *fp = fopen(path, "rb");
  if (fp == NULL) {
    fprintf(stderr, "Could not open '%s': %s\n", path, strerror(errno));
    return 2;
  }

  printf("\nFile: %s\n\n", path);

  /* ---- Step 1a: DOS header, "MZ" signature at offset 0 ---- */
  unsigned char dos[DOS_HEADER_SIZE];
  size_t got = fread(dos, 1, sizeof(dos), fp);
  if (got < sizeof(dos)) {
    printf("Result: NOT a PE file (only %lu bytes; a DOS header needs %d)\n",
           (unsigned long)got, DOS_HEADER_SIZE);
    fclose(fp);
    return 1;
  }

  int has_mz = (dos[0] == 'M' && dos[1] == 'Z');
  printf("DOS signature   offset 0x00   bytes: %02X %02X           %s\n",
         dos[0], dos[1],
         has_mz ? "\"MZ\"  OK" : "expected 4D 5A (\"MZ\")  MISSING");
  if (!has_mz) {
    printf("\nResult: NOT a PE file (no MZ signature)\n");
    fclose(fp);
    return 1;
  }

  /* ---- Step 2: e_lfanew, 4 bytes at 0x3C ---- */
  uint32_t e_lfanew = le32(dos + E_LFANEW_OFFSET);
  printf("e_lfanew        offset 0x3C   bytes: %02X %02X %02X %02X   = 0x%08X "
         "(%lu)\n",
         dos[0x3C], dos[0x3D], dos[0x3E], dos[0x3F], (unsigned)e_lfanew,
         (unsigned long)e_lfanew);

  /* ---- Step 1b: "PE\0\0" signature at e_lfanew ---- */
  unsigned char sig[4];
  if (e_lfanew > 0x7FFFFFFFu || fseek(fp, (long)e_lfanew, SEEK_SET) != 0 ||
      fread(sig, 1, sizeof(sig), fp) != sizeof(sig)) {
    printf("\nResult: NOT a PE file (e_lfanew points outside the file)\n");
    fclose(fp);
    return 1;
  }

  int has_pe = (memcmp(sig, "PE\0\0", 4) == 0);
  printf("PE signature    offset 0x%-3X  bytes: %02X %02X %02X %02X   %s\n",
         (unsigned)e_lfanew, sig[0], sig[1], sig[2], sig[3],
         has_pe ? "\"PE\\0\\0\"  OK"
                : "expected 50 45 00 00 (\"PE\\0\\0\")  MISSING");

  fclose(fp);

  if (!has_pe) {
    printf("\nResult: NOT a PE file (MZ is present, but no PE signature at "
           "e_lfanew)\n");
    return 1;
  }

  printf("\nResult: Valid PE file\n");
  return 0;
}
