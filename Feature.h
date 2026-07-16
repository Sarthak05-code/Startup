#ifndef Feature_h
#define Feature_h

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void panic(const char *message);

#define Panic(fmt, ...)                                                        \
  do {                                                                         \
    fprintf(stderr, "PANIC: " fmt "\n", ##__VA_ARGS__);                        \
    exit(1);                                                                   \
  } while (0)

/*
   extension_remover Macro:
   - filename: The input string (e.g., "family.txt")
   - output_buf: A char array to store the result
   - buf_size: The size of your output buffer to prevent overflows
*/
#define extension_remover(filename, output_buf, buf_size)                      \
  do {                                                                         \
    if ((filename) == NULL) {                                                  \
      Panic("extension_remover received a NULL filename");                     \
    }                                                                          \
    /* Find the LAST occurrence of '.' in the string */                        \
    const char *dot = strrchr((filename), '.');                                \
    size_t len;                                                                \
    if (dot != NULL) {                                                         \
      len = dot - (filename); /* Length up to the dot */                       \
    } else {                                                                   \
      len = strlen(filename); /* No dot found, copy the whole thing */         \
    }                                                                          \
    /* Ensure we don't overflow our output buffer */                           \
    if (len >= (buf_size)) {                                                   \
      len = (buf_size) - 1;                                                    \
    }                                                                          \
    strncpy((output_buf), (filename), len);                                    \
    (output_buf)[len] = '\0'; /* Manually null-terminate */                    \
  } while (0)

#endif
