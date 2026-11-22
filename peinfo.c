/*
 * peinfo.c - PE explorer, steps 1 to 4
 *
 *   1. Check the "MZ" and "PE\0\0" signatures
 *   2. Read e_lfanew (4 bytes at 0x3C): file offset of the PE header
 *   3. Parse the COFF header (20 bytes after "PE\0\0") into a struct
 *   4. Decode the fields: machine, timestamp, characteristics flags,
 *      optional-header magic (PE32 / PE32+) and subsystem
 *
 * Usage:  peinfo [file]        (prompts for a file name if omitted)
 * Exit code: 0 = valid PE, 1 = not a (complete) PE file, 2 = could not open
 * file
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define DOS_HEADER_SIZE 64
#define E_LFANEW_OFFSET 0x3C
#define COFF_HEADER_SIZE 20
#define OPT_PEEK_SIZE                                                          \
  70 /* optional header bytes 0..69: reaches Magic and Subsystem */
#define OPT_SUBSYSTEM_OFFSET 68

#define COUNT(a) (sizeof(a) / sizeof((a)[0]))

/* ---------- little-endian helpers (work on any machine) ---------- */

static uint16_t le16(const unsigned char *p) {
  return (uint16_t)(p[0] | (p[1] << 8));
}

static uint32_t le32(const unsigned char *p) {
  return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
         ((uint32_t)p[3] << 24);
}

/* ---------- the COFF header (step 3) ---------- */

typedef struct {
  uint16_t Machine;
  uint16_t NumberOfSections;
  uint32_t TimeDateStamp;
  uint32_t PointerToSymbolTable;
  uint32_t NumberOfSymbols;
  uint16_t SizeOfOptionalHeader;
  uint16_t Characteristics;
} CoffHeader;

/* Fill the struct field by field from the raw bytes. We do NOT fread() straight
 * into the struct: that only works if the compiler adds no padding and the CPU
 * is little-endian. Decoding each field by hand works everywhere. */
static void parse_coff(const unsigned char *b, CoffHeader *h) {
  h->Machine = le16(b + 0);
  h->NumberOfSections = le16(b + 2);
  h->TimeDateStamp = le32(b + 4);
  h->PointerToSymbolTable = le32(b + 8);
  h->NumberOfSymbols = le32(b + 12);
  h->SizeOfOptionalHeader = le16(b + 16);
  h->Characteristics = le16(b + 18);
}

/* ---------- lookup tables (step 4) ---------- */

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

/* Characteristics is a bitmask: each bit is a separate yes/no flag. */
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
  for (size_t i = 0; i < n; ++i)
    if (t[i].value == v)
      return t[i].name;
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
  time_t t = (time_t)ts; /* seconds since 1970-01-01 UTC */
  struct tm *tmv = gmtime(&t);
  char buf[64];
  if (tmv != NULL &&
      strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S UTC", tmv) > 0)
    printf("%s", buf);
  else
    printf("(invalid)");
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

  /* ---- Steps 1 and 2: DOS header, e_lfanew, PE signature ---- */
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

  uint32_t e_lfanew = le32(dos + E_LFANEW_OFFSET);
  printf("e_lfanew        offset 0x3C   bytes: %02X %02X %02X %02X   = 0x%08X "
         "(%lu)\n",
         dos[0x3C], dos[0x3D], dos[0x3E], dos[0x3F], (unsigned)e_lfanew,
         (unsigned long)e_lfanew);

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
  if (!has_pe) {
    printf("\nResult: NOT a PE file (MZ is present, but no PE signature at "
           "e_lfanew)\n");
    fclose(fp);
    return 1;
  }

  /* ---- Step 3: COFF header (the file position is right after "PE\0\0") ----
   */
  unsigned long coff_off = (unsigned long)e_lfanew + 4;
  unsigned char coff_raw[COFF_HEADER_SIZE];
  if (fread(coff_raw, 1, sizeof(coff_raw), fp) != sizeof(coff_raw)) {
    printf(
        "\nResult: NOT a complete PE file (it ends inside the COFF header)\n");
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
                         : "not marked executable (object file or other)";
  printf("  File type             %s\n", kind);

  /* ---- Step 4 (rest): optional header Magic and Subsystem ---- */
  if (h.SizeOfOptionalHeader >= OPT_PEEK_SIZE) {
    unsigned char opt[OPT_PEEK_SIZE];
    if (fread(opt, 1, sizeof(opt), fp) == sizeof(opt)) {
      uint16_t magic = le16(opt);
      uint16_t subsys = le16(opt + OPT_SUBSYSTEM_OFFSET);
      const char *fmt = magic == 0x10B   ? "PE32 (32-bit)"
                        : magic == 0x20B ? "PE32+ (64-bit)"
                        : magic == 0x107 ? "ROM image"
                                         : "unrecognized";
      const char *sname = lookup(SUBSYSTEMS, COUNT(SUBSYSTEMS), subsys);

      printf("\nOptional header (offset 0x%lX, %u bytes)\n",
             coff_off + COFF_HEADER_SIZE, (unsigned)h.SizeOfOptionalHeader);
      printf("  Magic                 0x%04X      %s\n", (unsigned)magic, fmt);
      printf("  Subsystem             %-10u  %s\n", (unsigned)subsys,
             sname ? sname : "unrecognized");
    } else {
      printf("\nResult: NOT a complete PE file (it ends inside the optional "
             "header)\n");
      fclose(fp);
      return 1;
    }
  } else if (h.SizeOfOptionalHeader == 0) {
    printf(
        "\nNo optional header (normal for object files, not for programs).\n");
  } else {
    printf("\nOptional header is only %u bytes, too short to read the "
           "subsystem.\n",
           (unsigned)h.SizeOfOptionalHeader);
  }

  fclose(fp);
  printf("\nResult: Valid PE file\n");
  return 0;
}
