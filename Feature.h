/* Feature.h — Windows-only C utility macros
 * Hardened for MSVC, MinGW, and Clang on Windows
 *
 * Fixes in this version:
 *   - TraceTime now uses GetProcessTimes (clock() is wall-clock on MSVC)
 *   - Removed cached QPC frequency to eliminate 32-bit torn-read race
 *   - str_copy and extension_remover use memmove for overlap safety
 *   - safe_realloc never frees the old pointer on failure
 *   - Windows-only compile guard
 */

#ifndef FEATURE_H
#define FEATURE_H

/* ─── Windows-only guard ─── */
#if !defined(_WIN32) && !defined(_WIN64)
#error                                                                         \
    "Feature.h is Windows-only. Use the cross-platform version for other platforms."
#endif

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>


/* ─── Compiler capability detection ─── */
#if defined(__GNUC__) || defined(__clang__)
#define FEATURE_HAS_STATEMENT_EXPRESSION 1
#define FEATURE_HAS_CLEANUP_ATTRIBUTE 1
#else
#define FEATURE_HAS_STATEMENT_EXPRESSION 0
#define FEATURE_HAS_CLEANUP_ATTRIBUTE 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ─── Internal helpers ─── */
#define _FEATURE_CONCAT_INNER(a, b) a##b
#define _FEATURE_CONCAT(a, b) _FEATURE_CONCAT_INNER(a, b)
#define _FEATURE_UNIQUE(prefix) _FEATURE_CONCAT(prefix, __LINE__)

/* ─── Filename helper ─── */
static inline const char *_feature_basename(const char *path) {
  if (path == NULL)
    return "<unknown>";
  const char *last_slash = strrchr(path, '/');
  const char *last_backslash = strrchr(path, '\\');
  const char *base = path;
  if (last_slash != NULL && last_slash >= base)
    base = last_slash + 1;
  if (last_backslash != NULL && last_backslash >= base)
    base = last_backslash + 1;
  return base;
}
#define __FILENAME__ (_feature_basename(__FILE__))

/* ─── Panic macros ─── */
#define panic(fmt, ...)                                                        \
  do {                                                                         \
    fprintf(stderr, "PANIC [%s:%d]: " fmt "\n", __FILENAME__, __LINE__,        \
            ##__VA_ARGS__);                                                    \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

#define panic_msg(msg) panic("%s", (msg))

/* ─── Extension stripper ─── */
#define extension_remover(filename, output_buf, buf_size)                      \
  do {                                                                         \
    const char *_feature_filename = (filename);                                \
    char *_feature_output = (output_buf);                                      \
    size_t _feature_buf_size = (buf_size);                                     \
                                                                               \
    if (_feature_filename == NULL) {                                           \
      panic_msg("extension_remover received a NULL filename");                 \
    }                                                                          \
    if (_feature_output == NULL) {                                             \
      panic_msg("extension_remover received a NULL output buffer");            \
    }                                                                          \
    if (_feature_buf_size == 0) {                                              \
      panic_msg("extension_remover received a zero-size buffer");              \
    }                                                                          \
    const char *_feature_dot = strrchr(_feature_filename, '.');                \
    size_t _feature_len = (_feature_dot != NULL)                               \
                              ? (size_t)(_feature_dot - _feature_filename)     \
                              : strlen(_feature_filename);                     \
    if (_feature_len >= _feature_buf_size) {                                   \
      panic("Output buffer too small: need %zu, have %zu", _feature_len + 1,   \
            _feature_buf_size);                                                \
    }                                                                          \
    memmove(_feature_output, _feature_filename, _feature_len);                 \
    _feature_output[_feature_len] = '\0';                                      \
  } while (0)

/* ─── CPU time tracer (FIXED: uses GetProcessTimes, not clock()) ─── */
#define TraceTime(block_name, block_code)                                      \
  do {                                                                         \
    FILETIME _feature_ft_create, _feature_ft_exit;                             \
    FILETIME _feature_ft_kernel_start, _feature_ft_kernel_end;                 \
    FILETIME _feature_ft_user_start, _feature_ft_user_end;                     \
    ULARGE_INTEGER _feature_kernel_start, _feature_kernel_end;                 \
    ULARGE_INTEGER _feature_user_start, _feature_user_end;                     \
                                                                               \
    GetProcessTimes(GetCurrentProcess(), &_feature_ft_create,                  \
                    &_feature_ft_exit, &_feature_ft_kernel_start,              \
                    &_feature_ft_user_start);                                  \
    block_code;                                                                \
    GetProcessTimes(GetCurrentProcess(), &_feature_ft_create,                  \
                    &_feature_ft_exit, &_feature_ft_kernel_end,                \
                    &_feature_ft_user_end);                                    \
                                                                               \
    _feature_kernel_start.LowPart = _feature_ft_kernel_start.dwLowDateTime;    \
    _feature_kernel_start.HighPart = _feature_ft_kernel_start.dwHighDateTime;  \
    _feature_kernel_end.LowPart = _feature_ft_kernel_end.dwLowDateTime;        \
    _feature_kernel_end.HighPart = _feature_ft_kernel_end.dwHighDateTime;      \
    double _feature_kernel = (double)(_feature_kernel_end.QuadPart -           \
                                      _feature_kernel_start.QuadPart) /        \
                             1e7;                                              \
                                                                               \
    _feature_user_start.LowPart = _feature_ft_user_start.dwLowDateTime;        \
    _feature_user_start.HighPart = _feature_ft_user_start.dwHighDateTime;      \
    _feature_user_end.LowPart = _feature_ft_user_end.dwLowDateTime;            \
    _feature_user_end.HighPart = _feature_ft_user_end.dwHighDateTime;          \
    double _feature_user =                                                     \
        (double)(_feature_user_end.QuadPart - _feature_user_start.QuadPart) /  \
        1e7;                                                                   \
                                                                               \
    fprintf(stdout,                                                            \
            "[PERF]: '%s' took %.6f seconds (CPU: kernel %.6f, user %.6f)\n",  \
            (block_name), _feature_kernel + _feature_user, _feature_kernel,    \
            _feature_user);                                                    \
  } while (0)

/* ─── Wall-clock time tracer (FIXED: no cached frequency) ─── */
#define TraceTimeWall(block_name, block_code)                                  \
  do {                                                                         \
    LARGE_INTEGER _feature_freq;                                               \
    if (!QueryPerformanceFrequency(&_feature_freq)) {                          \
      panic_msg("QueryPerformanceFrequency failed");                           \
    }                                                                          \
    LARGE_INTEGER _feature_start, _feature_end;                                \
    QueryPerformanceCounter(&_feature_start);                                  \
    block_code;                                                                \
    QueryPerformanceCounter(&_feature_end);                                    \
    double _feature_elapsed =                                                  \
        (double)(_feature_end.QuadPart - _feature_start.QuadPart) /            \
        _feature_freq.QuadPart;                                                \
    fprintf(stdout, "[PERF]: '%s' took %.6f seconds (wall)\n", (block_name),   \
            _feature_elapsed);                                                 \
  } while (0)

/* ─── Thread time tracer ─── */
#define TraceTimeThread(block_name, block_code)                                \
  do {                                                                         \
    FILETIME _feature_ft_create, _feature_ft_exit;                             \
    FILETIME _feature_ft_kernel_start, _feature_ft_kernel_end;                 \
    FILETIME _feature_ft_user_start, _feature_ft_user_end;                     \
    ULARGE_INTEGER _feature_kernel_start, _feature_kernel_end;                 \
    ULARGE_INTEGER _feature_user_start, _feature_user_end;                     \
                                                                               \
    GetThreadTimes(GetCurrentThread(), &_feature_ft_create, &_feature_ft_exit, \
                   &_feature_ft_kernel_start, &_feature_ft_user_start);        \
    block_code;                                                                \
    GetThreadTimes(GetCurrentThread(), &_feature_ft_create, &_feature_ft_exit, \
                   &_feature_ft_kernel_end, &_feature_ft_user_end);            \
                                                                               \
    _feature_kernel_start.LowPart = _feature_ft_kernel_start.dwLowDateTime;    \
    _feature_kernel_start.HighPart = _feature_ft_kernel_start.dwHighDateTime;  \
    _feature_kernel_end.LowPart = _feature_ft_kernel_end.dwLowDateTime;        \
    _feature_kernel_end.HighPart = _feature_ft_kernel_end.dwHighDateTime;      \
    double _feature_kernel = (double)(_feature_kernel_end.QuadPart -           \
                                      _feature_kernel_start.QuadPart) /        \
                             1e7;                                              \
                                                                               \
    _feature_user_start.LowPart = _feature_ft_user_start.dwLowDateTime;        \
    _feature_user_start.HighPart = _feature_ft_user_start.dwHighDateTime;      \
    _feature_user_end.LowPart = _feature_ft_user_end.dwLowDateTime;            \
    _feature_user_end.HighPart = _feature_ft_user_end.dwHighDateTime;          \
    double _feature_user =                                                     \
        (double)(_feature_user_end.QuadPart - _feature_user_start.QuadPart) /  \
        1e7;                                                                   \
                                                                               \
    fprintf(stdout,                                                            \
            "[PERF]: '%s' took %.6f seconds (kernel: %.6f, user: %.6f)\n",     \
            (block_name), _feature_kernel + _feature_user, _feature_kernel,    \
            _feature_user);                                                    \
  } while (0)

/* ─── Safe memory allocation ─── */
#if FEATURE_HAS_STATEMENT_EXPRESSION

#define safe_malloc(size)                                                      \
  ({                                                                           \
    size_t _feature_size = (size_t)(size);                                     \
    void *_feature_ptr = malloc(_feature_size);                                \
    if (_feature_ptr == NULL) {                                                \
      panic("malloc(%zu) failed", _feature_size);                              \
    }                                                                          \
    _feature_ptr;                                                              \
  })

#define safe_calloc(count, size)                                               \
  ({                                                                           \
    size_t _feature_count = (size_t)(count);                                   \
    size_t _feature_size = (size_t)(size);                                     \
    void *_feature_ptr = calloc(_feature_count, _feature_size);                \
    if (_feature_ptr == NULL) {                                                \
      panic("calloc(%zu, %zu) failed", _feature_count, _feature_size);         \
    }                                                                          \
    _feature_ptr;                                                              \
  })

#define safe_realloc(ptr, size)                                                \
  ({                                                                           \
    void *_feature_old_ptr = (void *)(ptr);                                    \
    size_t _feature_size = (size_t)(size);                                     \
    void *_feature_ptr = realloc(_feature_old_ptr, _feature_size);             \
    if (_feature_ptr == NULL && _feature_size > 0) {                           \
      panic("realloc(%p, %zu) failed (original pointer still valid)",          \
            _feature_old_ptr, _feature_size);                                  \
    }                                                                          \
    _feature_ptr;                                                              \
  })

#else /* MSVC */

static inline void *safe_malloc_impl(size_t size, const char *file, int line) {
  void *ptr = malloc(size);
  if (ptr == NULL) {
    fprintf(stderr, "PANIC [%s:%d]: malloc(%zu) failed\n",
            _feature_basename(file), line, size);
    exit(EXIT_FAILURE);
  }
  return ptr;
}

static inline void *safe_calloc_impl(size_t count, size_t size,
                                     const char *file, int line) {
  void *ptr = calloc(count, size);
  if (ptr == NULL) {
    fprintf(stderr, "PANIC [%s:%d]: calloc(%zu, %zu) failed\n",
            _feature_basename(file), line, count, size);
    exit(EXIT_FAILURE);
  }
  return ptr;
}

static inline void *safe_realloc_impl(void *ptr, size_t size, const char *file,
                                      int line) {
  void *new_ptr = realloc(ptr, size);
  if (new_ptr == NULL && size > 0) {
    fprintf(stderr,
            "PANIC [%s:%d]: realloc(%p, %zu) failed "
            "(original pointer still valid)\n",
            _feature_basename(file), line, ptr, size);
    exit(EXIT_FAILURE);
  }
  return new_ptr;
}

#define safe_malloc(size) safe_malloc_impl((size_t)(size), __FILE__, __LINE__)
#define safe_calloc(count, size)                                               \
  safe_calloc_impl((size_t)(count), (size_t)(size), __FILE__, __LINE__)
#define safe_realloc(ptr, size)                                                \
  safe_realloc_impl((void *)(ptr), (size_t)(size), __FILE__, __LINE__)

#endif

/* ─── Safe string copy (FIXED: memmove for overlap safety) ─── */
#define str_copy(dest, src, dest_size)                                         \
  do {                                                                         \
    char *_feature_dest = (dest);                                              \
    const char *_feature_src = (src);                                          \
    size_t _feature_dest_size = (dest_size);                                   \
                                                                               \
    if (_feature_dest == NULL || _feature_src == NULL) {                       \
      panic_msg("str_copy received NULL pointer");                             \
    }                                                                          \
    if (_feature_dest_size == 0) {                                             \
      panic_msg("str_copy received zero-size destination buffer");             \
    }                                                                          \
    size_t _feature_src_len = strlen(_feature_src);                            \
    if (_feature_src_len >= _feature_dest_size) {                              \
      panic("str_copy buffer too small: need %zu, have %zu",                   \
            _feature_src_len + 1, _feature_dest_size);                         \
    }                                                                          \
    memmove(_feature_dest, _feature_src, _feature_src_len);                    \
    _feature_dest[_feature_src_len] = '\0';                                    \
  } while (0)

/* ─── Scoped cleanup ─── */
#if FEATURE_HAS_CLEANUP_ATTRIBUTE

#define defer_free(ptr)                                                        \
  void *_FEATURE_UNIQUE(_feature_cleanup_)                                     \
      __attribute__((cleanup(_feature_auto_free))) = (ptr)

static inline void _feature_auto_free(void *p) {
  void **pp = (void **)p;
  if (*pp != NULL) {
    free(*pp);
    *pp = NULL;
  }
}

#else /* MSVC fallback */

#define _FEATURE_DEFER_MAX 32

__declspec(thread) static void *_feature_defer_stack[_FEATURE_DEFER_MAX] = {0};
__declspec(thread) static int _feature_defer_count = 0;

#define defer_free(ptr)                                                        \
  do {                                                                         \
    void *_feature_p = (ptr);                                                  \
    if (_feature_defer_count < _FEATURE_DEFER_MAX) {                           \
      _feature_defer_stack[_feature_defer_count++] = _feature_p;               \
    } else {                                                                   \
      fprintf(stderr, "WARNING: defer_free stack overflow at %s:%d\n",         \
              __FILENAME__, __LINE__);                                         \
    }                                                                          \
  } while (0)

#define defer_cleanup()                                                        \
  do {                                                                         \
    while (_feature_defer_count > 0) {                                         \
      void *_feature_p = _feature_defer_stack[--_feature_defer_count];         \
      if (_feature_p != NULL) {                                                \
        free(_feature_p);                                                      \
      }                                                                        \
    }                                                                          \
  } while (0)

#endif

#ifdef __cplusplus
}
#endif

#endif /* FEATURE_H */
