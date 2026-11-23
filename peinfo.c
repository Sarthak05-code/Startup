
/*
 * peinfo.c - PE explorer, steps 1 to 5
 *
 *   1. Check the "MZ" and "PE\0\0" signatures
 *   2. Read e_lfanew (4 bytes at 0x3C)
 *   3. Parse the COFF header
 *   4. Decode machine, timestamp and characteristics
 *   5. Parse the Optional Header (PE32 and PE32+)
 *
 * Usage: peinfo [file]
 */

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DOS_HEADER_SIZE 64
#define E_LFANEW_OFFSET 0x3C
#define COFF_HEADER_SIZE 20

#define OPT_MIN_SIZE 70
#define OPT_SUBSYSTEM_OFFSET 68

#define COUNT(a) (sizeof(a) / sizeof((a)[0]))

/* ---------- little-endian helpers ---------- */

static uint16_t le16(const unsigned char *p) {
  return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t le32(const unsigned char *p) {
  return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
         ((uint32_t)p[3] << 24);
}

static uint64_t le64(const unsigned char *p) {
  return (uint64_t)p[0] | ((uint64_t)p[1] << 8) | ((uint64_t)p[2] << 16) |
         ((uint64_t)p[3] << 24) | ((uint64_t)p[4] << 32) |
         ((uint64_t)p[5] << 40) | ((uint64_t)p[6] << 48) |
         ((uint64_t)p[7] << 56);
}

/* ---------- COFF header ---------- */

typedef struct {
  uint16_t Machine;
  uint16_t NumberOfSections;
  uint32_t TimeDateStamp;
  uint32_t PointerToSymbolTable;
  uint32_t NumberOfSymbols;
  uint16_t SizeOfOptionalHeader;
  uint16_t Characteristics;
} CoffHeader;

static void parse_coff(const unsigned char *b, CoffHeader *h) {
  h->Machine = le16(b + 0);
  h->NumberOfSections = le16(b + 2);
  h->TimeDateStamp = le32(b + 4);
  h->PointerToSymbolTable = le32(b + 8);
  h->NumberOfSymbols = le32(b + 12);
  h->SizeOfOptionalHeader = le16(b + 16);
  h->Characteristics = le16(b + 18);
}

/* ---------- lookup tables ---------- */

typedef struct {
  uint16_t value;
  const char *name;
} Name16;

static const Name16 MACHINES[] = {
    {0x0000, "any / unknown"},   {0x014C, "x86 (i386, 32-bit)"},
    {0x8664, "x64 (AMD64)"},     {0x01C0, "ARM"},
    {0x01C4, "ARM Thumb-2"},     {0xAA64, "ARM64"},
    {0x0200, "Itanium (IA-64)"}, {0x5032, "RISC-V 32"},
    {0x5064, "RISC-V 64"},
};

static const Name16 SUBSYSTEMS[] = {
    {0, "Unknown"},
    {1, "Native (no subsystem)"},
    {2, "Windows GUI"},
    {3, "Windows console (CUI)"},
    {5, "OS/2 console"},
    {7, "POSIX console"},
    {9, "Windows CE GUI"},
    {10, "EFI application"},
    {11, "EFI boot service driver"},
    {12, "EFI runtime driver"},
    {13, "EFI ROM"},
    {14, "Xbox"},
    {16, "Windows boot application"},
};

static const Name16 CHARACTERISTICS[] = {
    {0x0001, "RELOCS_STRIPPED"},
    {0x0002, "EXECUTABLE_IMAGE"},
    {0x0004, "LINE_NUMS_STRIPPED"},
    {0x0008, "LOCAL_SYMS_STRIPPED"},
    {0x0010, "AGGRESSIVE_WS_TRIM"},
    {0x0020, "LARGE_ADDRESS_AWARE"},
    {0x0080, "BYTES_REVERSED_LO"},
    {0x0100, "32BIT_MACHINE"},
    {0x0200, "DEBUG_STRIPPED"},
    {0x0400, "REMOVABLE_RUN_FROM_SWAP"},
    {0x0800, "NET_RUN_FROM_SWAP"},
    {0x1000, "SYSTEM"},
    {0x2000, "DLL"},
    {0x4000, "UP_SYSTEM_ONLY"},
    {0x8000, "BYTES_REVERSED_HI"},
};

#define IMAGE_FILE_EXECUTABLE_IMAGE 0x0002
#define IMAGE_FILE_DLL 0x2000

static const char *lookup(const Name16 *t, size_t n, uint16_t v) {
  for (size_t i = 0; i < n; ++i) {
    if (t[i].value == v)
      return t[i].name;
  }
  return NULL;
}

static void print_flags(uint16_t c) {
  uint16_t rest = c;
  int first = 1;

  for (size_t i = 0; i < COUNT(CHARACTERISTICS); ++i) {
    if (c & CHARACTERISTICS[i].value) {
      printf("%s%s", first ? "" : " | ", CHARACTERISTICS[i].name);

      first = 0;
      rest = (uint16_t)(rest & ~CHARACTERISTICS[i].value);
    }
  }

  if (rest)
    printf("%sunknown bits 0x%04X", first ? "" : " | ", (unsigned)rest);
  else if (first)
    printf("(none)");
}

static void print_timestamp(uint32_t ts) {
  if (ts == 0) {
    printf("(0: not set)");
    return;
  }

  time_t t = (time_t)ts;
  struct tm *tmv = gmtime(&t);
  char buf[64];

  if (tmv != NULL &&
      strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S UTC", tmv) > 0)
    printf("%s", buf);
  else
    printf("(invalid)");
}

/* ---------- Optional Header parser ---------- */

static int parse_optional_header(const unsigned char *opt, size_t size) {
  if (size < OPT_MIN_SIZE) {
    printf("\nOptional Header is too short (%zu bytes).\n", size);
    return 0;
  }

  uint16_t magic = le16(opt);

  printf("\nOptional Header\n");
  printf("  Magic                 0x%04X", (unsigned)magic);

  if (magic == 0x10B) {
    printf("      PE32 (32-bit)\n");

    uint32_t entry_point = le32(opt + 16);
    uint32_t image_base = le32(opt + 28);
    uint32_t section_alignment = le32(opt + 32);
    uint32_t file_alignment = le32(opt + 36);
    uint32_t size_of_image = le32(opt + 56);
    uint32_t size_of_headers = le32(opt + 60);
    uint32_t checksum = le32(opt + 64);
    uint16_t subsystem = le16(opt + OPT_SUBSYSTEM_OFFSET);

    printf("  AddressOfEntryPoint   0x%08X\n", (unsigned)entry_point);
    printf("  ImageBase             0x%08X\n", (unsigned)image_base);
    printf("  SectionAlignment      0x%08X (%u)\n", (unsigned)section_alignment,
           (unsigned)section_alignment);
    printf("  FileAlignment         0x%08X (%u)\n", (unsigned)file_alignment,
           (unsigned)file_alignment);
    printf("  SizeOfImage           0x%08X (%u)\n", (unsigned)size_of_image,
           (unsigned)size_of_image);
    printf("  SizeOfHeaders         0x%08X (%u)\n", (unsigned)size_of_headers,
           (unsigned)size_of_headers);
    printf("  CheckSum              0x%08X\n", (unsigned)checksum);

    const char *sname = lookup(SUBSYSTEMS, COUNT(SUBSYSTEMS), subsystem);

    printf("  Subsystem             %-10u %s\n", (unsigned)subsystem,
           sname ? sname : "unrecognized");

  } else if (magic == 0x20B) {
    printf("      PE32+ (64-bit)\n");

    uint32_t entry_point = le32(opt + 16);
    uint64_t image_base = le64(opt + 24);
    uint32_t section_alignment = le32(opt + 32);
    uint32_t file_alignment = le32(opt + 36);
    uint32_t size_of_image = le32(opt + 56);
    uint32_t size_of_headers = le32(opt + 60);
    uint32_t checksum = le32(opt + 64);
    uint16_t subsystem = le16(opt + OPT_SUBSYSTEM_OFFSET);

    printf("  AddressOfEntryPoint   0x%08X\n", (unsigned)entry_point);
    printf("  ImageBase             0x%016llX\n",
           (unsigned long long)image_base);
    printf("  SectionAlignment      0x%08X (%u)\n", (unsigned)section_alignment,
           (unsigned)section_alignment);
    printf("  FileAlignment         0x%08X (%u)\n", (unsigned)file_alignment,
           (unsigned)file_alignment);
    printf("  SizeOfImage           0x%08X (%u)\n", (unsigned)size_of_image,
           (unsigned)size_of_image);
    printf("  SizeOfHeaders         0x%08X (%u)\n", (unsigned)size_of_headers,
           (unsigned)size_of_headers);
    printf("  CheckSum              0x%08X\n", (unsigned)checksum);

    const char *sname = lookup(SUBSYSTEMS, COUNT(SUBSYSTEMS), subsystem);

    printf("  Subsystem             %-10u %s\n", (unsigned)subsystem,
           sname ? sname : "unrecognized");

  } else {
    printf("      Unsupported Optional Header format\n");
    return 0;
  }

  return 1;
}

/* ---------- main ---------- */

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

  /* ---- Steps 1 and 2: DOS Header and PE signature ---- */

  unsigned char dos[DOS_HEADER_SIZE];

  size_t got = fread(dos, 1, sizeof(dos), fp);

  if (got < sizeof(dos)) {
    printf("Result: NOT a PE file (incomplete DOS header)\n");
    fclose(fp);
    return 1;
  }

  int has_mz = (dos[0] == 'M' && dos[1] == 'Z');

  printf("DOS signature   offset 0x00   bytes: %02X %02X   %s\n", dos[0],
         dos[1], has_mz ? "\"MZ\" OK" : "MZ signature missing");

  if (!has_mz) {
    printf("\nResult: NOT a PE file\n");
    fclose(fp);
    return 1;
  }

  uint32_t e_lfanew = le32(dos + E_LFANEW_OFFSET);

  printf("e_lfanew        offset 0x3C   = 0x%08X (%lu)\n", (unsigned)e_lfanew,
         (unsigned long)e_lfanew);

  unsigned char sig[4];

  if (e_lfanew > 0x7FFFFFFFu || fseek(fp, (long)e_lfanew, SEEK_SET) != 0 ||
      fread(sig, 1, sizeof(sig), fp) != sizeof(sig)) {

    printf("\nResult: NOT a PE file (invalid PE header offset)\n");
    fclose(fp);
    return 1;
  }

  int has_pe = (memcmp(sig, "PE\0\0", 4) == 0);

  printf("PE signature    offset 0x%08X   %s\n", (unsigned)e_lfanew,
         has_pe ? "\"PE\\0\\0\" OK" : "PE signature missing");

  if (!has_pe) {
    printf("\nResult: NOT a PE file\n");
    fclose(fp);
    return 1;
  }

  /* ---- Step 3: COFF Header ---- */

  unsigned long coff_off = (unsigned long)e_lfanew + 4;
  unsigned char coff_raw[COFF_HEADER_SIZE];

  if (fread(coff_raw, 1, sizeof(coff_raw), fp) != sizeof(coff_raw)) {
    printf("\nResult: Incomplete COFF header\n");
    fclose(fp);
    return 1;
  }

  CoffHeader h;
  parse_coff(coff_raw, &h);

  printf("\nCOFF header (offset 0x%lX, %d bytes)\n", coff_off,
         COFF_HEADER_SIZE);

  const char *mname = lookup(MACHINES, COUNT(MACHINES), h.Machine);

  printf("  Machine               0x%04X      %s\n", (unsigned)h.Machine,
         mname ? mname : "unrecognized");

  printf("  NumberOfSections      %u\n", (unsigned)h.NumberOfSections);

  printf("  TimeDateStamp         0x%08X  ", (unsigned)h.TimeDateStamp);

  print_timestamp(h.TimeDateStamp);
  printf("\n");

  printf("  PointerToSymbolTable  0x%08X\n", (unsigned)h.PointerToSymbolTable);

  printf("  NumberOfSymbols       %lu\n", (unsigned long)h.NumberOfSymbols);

  printf("  SizeOfOptionalHeader  %u (0x%X)\n",
         (unsigned)h.SizeOfOptionalHeader, (unsigned)h.SizeOfOptionalHeader);

  printf("  Characteristics       0x%04X      ", (unsigned)h.Characteristics);

  print_flags(h.Characteristics);
  printf("\n");

  const char *kind = (h.Characteristics & IMAGE_FILE_DLL) ? "DLL (library)"
                     : (h.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE)
                         ? "EXE (program)"
                         : "not marked executable";

  printf("  File type             %s\n", kind);

  /* ---- Step 5: Complete Optional Header ---- */

  if (h.SizeOfOptionalHeader == 0) {
    printf("\nNo Optional Header is present.\n");
    fclose(fp);
    printf("\nResult: PE/COFF header parsed; no Optional Header\n");
    return 0;
  }

  unsigned char *opt = malloc(h.SizeOfOptionalHeader);

  if (opt == NULL) {
    fprintf(stderr, "Memory allocation failed.\n");
    fclose(fp);
    return 2;
  }

  if (fread(opt, 1, h.SizeOfOptionalHeader, fp) != h.SizeOfOptionalHeader) {

    printf("\nResult: Incomplete Optional Header\n");

    free(opt);
    fclose(fp);
    return 1;
  }

  printf("\nOptional Header (offset 0x%lX, %u bytes)\n",
         coff_off + COFF_HEADER_SIZE, (unsigned)h.SizeOfOptionalHeader);

  int parsed = parse_optional_header(opt, h.SizeOfOptionalHeader);

  free(opt);
  fclose(fp);

  if (!parsed) {
    printf("\nResult: Optional Header could not be fully interpreted\n");
    return 1;
  }

  printf("\nResult: PE headers parsed successfully\n");
  return 0;
}
