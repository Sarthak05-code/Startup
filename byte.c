/*
 * byte.c - small hex dumper
 *
 * Usage:  dump [-n] [file] [offset] [length]
 *   file    file to dump (prompted for if omitted)
 *   offset  where to start, decimal or 0x-hex (default 0)
 *   length  how many bytes to show (default: to end of file)
 *   -n      no color (color is also off automatically when piped)
 *
 * Examples:
 *   dump byte.exe
 *   dump byte.exe 0x80 64
 *   dump -n byte.exe 0 512 > head.txt
 */

#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define SEEK64(f, o) _fseeki64((f), (long long)(o), SEEK_SET)
#define SEEKEND64(f) _fseeki64((f), 0, SEEK_END)
#define TELL64(f) _ftelli64(f)
#define STDOUT_IS_TTY() _isatty(_fileno(stdout))
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#else
#include <sys/types.h>
#include <unistd.h>
#define SEEK64(f, o) fseeko((f), (off_t)(o), SEEK_SET)
#define SEEKEND64(f) fseeko((f), 0, SEEK_END)
#define TELL64(f) ((long long)ftello(f))
#define STDOUT_IS_TTY() isatty(fileno(stdout))
#endif

#define BYTES_PER_LINE 16
#define CHUNK_SIZE 65536 /* must be a multiple of BYTES_PER_LINE */

#define C_RESET "\x1b[0m"
#define C_DIM "\x1b[90m"    /* 00 bytes */
#define C_GREEN "\x1b[32m"  /* printable ASCII */
#define C_YELLOW "\x1b[33m" /* everything else */

static const char HEX[] = "0123456789ABCDEF";

static int is_printable(unsigned char b) { return b >= 0x20 && b < 0x7F; }

static const char *color_for(unsigned char b) {
  if (b == 0x00)
    return C_DIM;
  if (is_printable(b))
    return C_GREEN;
  return C_YELLOW;
}

static char *put_str(char *p, const char *s) {
  while (*s)
    *p++ = *s++;
  return p;
}

/* Builds one output row into `out`; returns its length (no NUL written). */
static size_t format_line(char *out, unsigned long long off,
                          const unsigned char *b, size_t n, int color) {
  char *p = out;

  int digits = off > 0xFFFFFFFFULL ? 16 : 8;
  for (int d = digits - 1; d >= 0; --d)
    *p++ = HEX[(off >> (4 * d)) & 0xF];
  *p++ = ' ';
  *p++ = ' ';

  for (size_t i = 0; i < n; ++i) {
    if (color)
      p = put_str(p, color_for(b[i]));
    *p++ = HEX[b[i] >> 4];
    *p++ = HEX[b[i] & 0xF];
    if (color)
      p = put_str(p, C_RESET);
    *p++ = ' ';
  }
  /* each missing byte takes the same 3 columns as "XX " */
  for (size_t i = n; i < BYTES_PER_LINE; ++i) {
    *p++ = ' ';
    *p++ = ' ';
    *p++ = ' ';
  }
  *p++ = ' ';

  for (size_t i = 0; i < n; ++i) {
    char c = is_printable(b[i]) ? (char)b[i] : '.';
    if (color)
      p = put_str(p, color_for(b[i]));
    *p++ = c;
    if (color)
      p = put_str(p, C_RESET);
  }
  *p++ = '\n';
  return (size_t)(p - out);
}

static int parse_u64(const char *s, unsigned long long *out) {
  char *end;
  if (s[0] == '\0' || s[0] == '-')
    return 0;
  errno = 0;
  unsigned long long v =
      strtoull(s, &end, 0); /* base 0: 0x.. hex, 0.. octal, else decimal */
  if (end == s || *end != '\0' || errno == ERANGE)
    return 0;
  *out = v;
  return 1;
}

static void usage(const char *prog) {
  fprintf(stderr, "Usage: %s [-n] [file] [offset] [length]\n", prog);
  fprintf(stderr,
          "  offset/length: decimal or 0x-hex, e.g. %s file.exe 0x80 64\n",
          prog);
}

int main(int argc, char **argv) {
  int use_color = 1;
  int argi = 1;

  if (argi < argc && strcmp(argv[argi], "-n") == 0) {
    use_color = 0;
    argi++;
  }

  /* file name: argument if given, otherwise prompt like the old version */
  char prompt_name[512];
  const char *path;
  if (argi < argc) {
    path = argv[argi++];
  } else {
    printf("Enter filename: ");
    if (fgets(prompt_name, sizeof(prompt_name), stdin) == NULL) {
      fprintf(stderr, "Input error.\n");
      return 1;
    }
    prompt_name[strcspn(prompt_name, "\r\n")] = '\0';
    path = prompt_name;
  }

  unsigned long long start = 0, req_len = 0;
  int has_len = 0;

  if (argi < argc) {
    if (!parse_u64(argv[argi], &start)) {
      fprintf(stderr, "Bad offset: '%s'\n", argv[argi]);
      usage(argv[0]);
      return 1;
    }
    argi++;
  }
  if (argi < argc) {
    if (!parse_u64(argv[argi], &req_len)) {
      fprintf(stderr, "Bad length: '%s'\n", argv[argi]);
      usage(argv[0]);
      return 1;
    }
    has_len = 1;
    argi++;
  }
  if (argi < argc) {
    fprintf(stderr, "Too many arguments.\n");
    usage(argv[0]);
    return 1;
  }

  FILE *fp = fopen(path, "rb");
  if (fp == NULL) {
    fprintf(stderr, "Could not open '%s': %s\n", path, strerror(errno));
    return 1;
  }

  if (SEEKEND64(fp) != 0) {
    fprintf(stderr, "Could not determine size of '%s'.\n", path);
    fclose(fp);
    return 1;
  }
  long long size_ll = TELL64(fp);
  if (size_ll < 0) {
    fprintf(stderr, "Could not determine size of '%s'.\n", path);
    fclose(fp);
    return 1;
  }
  unsigned long long size = (unsigned long long)size_ll;

  if (start > size) {
    fprintf(stderr,
            "Offset 0x%llX is past the end of the file (size 0x%llX).\n", start,
            size);
    fclose(fp);
    return 1;
  }
  unsigned long long remaining = size - start;
  unsigned long long to_read =
      (has_len && req_len < remaining) ? req_len : remaining;

  if (SEEK64(fp, start) != 0) {
    fprintf(stderr, "Could not seek to offset 0x%llX.\n", start);
    fclose(fp);
    return 1;
  }

  /* color only on a real terminal; on Windows turn on ANSI handling first */
  if (use_color && !STDOUT_IS_TTY())
    use_color = 0;
#ifdef _WIN32
  if (use_color) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (h == INVALID_HANDLE_VALUE || !GetConsoleMode(h, &mode) ||
        !SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
      use_color = 0;
    }
  }
#endif

  setvbuf(stdout, NULL, _IOFBF, 1 << 16);

  printf("\n%-8s  %-*s %s\n", "Offset", BYTES_PER_LINE * 3, "Hexadecimal",
         "ASCII");
  for (int i = 0; i < 10 + BYTES_PER_LINE * 3 + 1 + BYTES_PER_LINE; ++i)
    putchar('-');
  putchar('\n');

  static unsigned char buf[CHUNK_SIZE];
  char line[1024];
  unsigned long long pos = start, left = to_read;

  while (left > 0) {
    size_t want = left < CHUNK_SIZE ? (size_t)left : CHUNK_SIZE;
    size_t got = fread(buf, 1, want, fp);
    if (got == 0)
      break;

    for (size_t i = 0; i < got; i += BYTES_PER_LINE) {
      size_t n = got - i < BYTES_PER_LINE ? got - i : BYTES_PER_LINE;
      size_t len = format_line(line, pos + i, buf + i, n, use_color);
      fwrite(line, 1, len, stdout);
    }
    pos += got;
    left -= got;
  }

  if (ferror(fp))
    fprintf(stderr, "Read error while dumping '%s'.\n", path);

  fflush(stdout);
  fclose(fp);
  return 0;
}
