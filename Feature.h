#ifndef Feature_h
#define Feature_h

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

/* ─── Filename helper ─── */
#define __FILENAME__                                                           \
  (strrchr(__FILE__, '\\')                                                     \
       ? strrchr(__FILE__, '\\') + 1                                           \
       : (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__))

/* ─── Panic macro ─── */
#define panic(fmt, ...)                                                        \
  do {                                                                         \
    fprintf(stderr, "PANIC [%s:%d]: " fmt "\n", __FILENAME__, __LINE__,        \
            ##__VA_ARGS__);                                                    \
    exit(1);                                                                   \
  } while (0)

/* ─── Extension stripper ─── */
#define extension_remover(filename, output_buf, buf_size)                      \
  do {                                                                         \
    if ((filename) == NULL) {                                                  \
      panic("extension_remover received a NULL filename");                     \
    }                                                                          \
    if ((output_buf) == NULL) {                                                \
      panic("extension_remover received a NULL output buffer");                 \
    }                                                                          \
    const char *dot = strrchr((filename), '.');                                \
    size_t len = (dot != NULL) ? (size_t)(dot - (filename)) : strlen(filename); \
    if (len >= (buf_size)) {                                                   \
      panic("Output buffer too small: need %zu, have %zu",                     \
            len + 1, (size_t)(buf_size));                                      \
    }                                                                          \
    memcpy((output_buf), (filename), len);                                     \
    (output_buf)[len] = '\0';                                                  \
  } while (0)

/* ─── CPU time tracer ─── */
#define TraceTime(block_name, block_code)                                      \
  do {                                                                         \
    clock_t start = clock();                                                   \
    block_code;                                                                \
    clock_t end = clock();                                                     \
    double cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;                \
    fprintf(stdout, "[PERF]: '%s' took %.6f seconds (CPU)\n",                 \
            block_name, cpu_time);                                             \
  } while (0)

/* ─── Wall-clock time tracer (Windows) ─── */
#define TraceTimeWall(block_name, block_code)                                  \
  do {                                                                         \
    LARGE_INTEGER freq, start, end;                                            \
    QueryPerformanceFrequency(&freq);                                          \
    QueryPerformanceCounter(&start);                                            \
    block_code;                                                                \
    QueryPerformanceCounter(&end);                                            \
    double elapsed = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart; \
    fprintf(stdout, "[PERF]: '%s' took %.6f seconds (wall)\n",               \
            block_name, elapsed);                                              \
  } while (0)

/* ─── High-res CPU tracer (Windows thread time) ─── */
#define TraceTimeThread(block_name, block_code)                                \
  do {                                                                         \
    FILETIME ft_create, ft_exit, ft_kernel, ft_user;                          \
    FILETIME ft_start_kernel, ft_start_user;                                   \
    FILETIME ft_end_kernel, ft_end_user;                                       \
    GetThreadTimes(GetCurrentThread(), &ft_create, &ft_exit,                  \
                   &ft_start_kernel, &ft_start_user);                           \
    block_code;                                                                \
    GetThreadTimes(GetCurrentThread(), &ft_create, &ft_exit,                  \
                   &ft_end_kernel, &ft_end_user);                              \
    ULARGE_INTEGER start, end;                                                 \
    start.LowPart = ft_start_kernel.dwLowDateTime;                             \
    start.HighPart = ft_start_kernel.dwHighDateTime;                           \
    end.LowPart = ft_end_kernel.dwLowDateTime;                                 \
    end.HighPart = ft_end_kernel.dwHighDateTime;                               \
    double kernel = (double)(end.QuadPart - start.QuadPart) / 1e7;           \
    start.LowPart = ft_start_user.dwLowDateTime;                               \
    start.HighPart = ft_start_user.dwHighDateTime;                             \
    end.LowPart = ft_end_user.dwLowDateTime;                                   \
    end.HighPart = ft_end_user.dwHighDateTime;                                 \
    double user = (double)(end.QuadPart - start.QuadPart) / 1e7;             \
    fprintf(stdout, "[PERF]: '%s' took %.6f seconds (kernel: %.6f, user: %.6f)\n", \
            block_name, kernel + user, kernel, user);                          \
  } while (0)

/* ─── Safe memory allocation (GNU statement expression) ─── */
#define safe_malloc(size)                                                      \
  ({                                                                           \
    void *_ptr = malloc(size);                                                 \
    if (_ptr == NULL) {                                                        \
      panic("malloc(%zu) failed", (size_t)(size));                             \
    }                                                                          \
    _ptr;                                                                      \
  })

#define safe_calloc(count, size)                                               \
  ({                                                                           \
    void *_ptr = calloc(count, size);                                          \
    if (_ptr == NULL) {                                                        \
      panic("calloc(%zu, %zu) failed", (size_t)(count), (size_t)(size));       \
    }                                                                          \
    _ptr;                                                                      \
  })

#define safe_realloc(ptr, size)                                                \
  ({                                                                           \
    void *_ptr = realloc(ptr, size);                                           \
    if (_ptr == NULL) {                                                        \
      panic("realloc(%zu) failed", (size_t)(size));                           \
    }                                                                          \
    _ptr;                                                                      \
  })

/* ─── Safe string copy ─── */
#define str_copy(dest, src, dest_size)                                         \
  do {                                                                         \
    if ((dest) == NULL || (src) == NULL) {                                     \
      panic("str_copy received NULL pointer");                                 \
    }                                                                          \
    size_t _src_len = strlen(src);                                             \
    if (_src_len >= (dest_size)) {                                             \
      panic("str_copy buffer too small: need %zu, have %zu",                   \
            _src_len + 1, (size_t)(dest_size));                                \
    }                                                                          \
    memcpy((dest), (src), _src_len + 1);                                       \
  } while (0)

/* ─── Scoped cleanup (GNU cleanup attribute) ─── */
#define defer_free(ptr)                                                        \
  void *_CONCAT(_cleanup_, __LINE__) __attribute__((cleanup(_auto_free))) = (ptr)

/* Helper: concat tokens for unique variable names */
#define _CONCAT(a, b) a##b

/* The cleanup handler receives a pointer to the variable */
static inline void _auto_free(void *p) {
  void **pp = (void **)p;
  if (*pp != NULL) {
    free(*pp);
    *pp = NULL;
  }
}

#endif