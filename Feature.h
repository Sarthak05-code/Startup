/* Feature.h — Cross-platform C utility macros
 * Improved version: portable, safe, and hardened against common pitfalls
 *
 * Changes from original:
 *   - Fixed safe_realloc double-free bug (CRITICAL)
 *   - Fixed extension_remover off-by-one for root paths
 *   - Replaced deprecated gettimeofday with clock_gettime(CLOCK_MONOTONIC) on POSIX
 *   - Added macOS fallback using mach_absolute_time (clock_gettime unavailable pre-10.12)
 *   - Added MSVC-compatible safe_malloc/safe_calloc/safe_realloc fallbacks
 *   - Improved defer_free MSVC fallback (actually tracks pointers)
 *   - Added thread-safe QPC frequency initialization on Windows
 *   - Cleaner TraceTimeThread variable handling (no reuse)
 *   - Added explicit feature detection macros
 *   - All allocation macros work on MSVC, GCC, and Clang
 *   - Proper _POSIX_C_SOURCE feature test macro for clock_gettime visibility
 */

#ifndef FEATURE_H
#define FEATURE_H

/* Must define feature test macros BEFORE any system headers on POSIX */
#if !defined(FEATURE_PLATFORM_WINDOWS)
  #if defined(_WIN32) || defined(_WIN64)
    #define FEATURE_PLATFORM_WINDOWS 1
  #else
    #define FEATURE_PLATFORM_WINDOWS 0
  #endif
#endif

#if !defined(FEATURE_PLATFORM_MACOS)
  #if defined(__APPLE__) && defined(__MACH__)
    #define FEATURE_PLATFORM_MACOS 1
  #else
    #define FEATURE_PLATFORM_MACOS 0
  #endif
#endif

/* For clock_gettime and CLOCK_MONOTONIC on POSIX systems */
#if !FEATURE_PLATFORM_WINDOWS && !FEATURE_PLATFORM_MACOS
  #if !defined(_POSIX_C_SOURCE)
    #define _POSIX_C_SOURCE 200809L
  #elif _POSIX_C_SOURCE < 199309L
    #undef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 200809L
  #endif
#endif

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

/* ─── Platform-specific includes ─── */
#if FEATURE_PLATFORM_WINDOWS
  #include <windows.h>
#elif FEATURE_PLATFORM_MACOS
  #include <mach/mach_time.h>
#else
  #include <unistd.h>
  #include <sys/time.h>
#endif

/* ─── Compiler capability detection ─── */
#if defined(__GNUC__) || defined(__clang__)
  #define FEATURE_HAS_STATEMENT_EXPRESSION 1
#else
  #define FEATURE_HAS_STATEMENT_EXPRESSION 0
#endif

#if defined(__GNUC__) || defined(__clang__)
  #define FEATURE_HAS_CLEANUP_ATTRIBUTE 1
#else
  #define FEATURE_HAS_CLEANUP_ATTRIBUTE 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ─── Internal helpers (do not use directly) ─── */

/* Concatenate tokens with proper macro expansion order */
#define _FEATURE_CONCAT_INNER(a, b) a##b
#define _FEATURE_CONCAT(a, b) _FEATURE_CONCAT_INNER(a, b)

/* Generate a unique identifier using the current line number */
#define _FEATURE_UNIQUE(prefix) _FEATURE_CONCAT(prefix, __LINE__)

/* ─── Filename helper (evaluates __FILE__ once) ─── */
static inline const char* _feature_basename(const char* path) {
    if (path == NULL) return "<unknown>";
    const char* last_slash = strrchr(path, '/');
    const char* last_backslash = strrchr(path, '\\');
    const char* base = path;
    /* FIXED: >= instead of > to handle root paths like "/file.txt" */
    if (last_slash != NULL && last_slash >= base) base = last_slash + 1;
    if (last_backslash != NULL && last_backslash >= base) base = last_backslash + 1;
    return base;
}
#define __FILENAME__ (_feature_basename(__FILE__))

/* ─── Panic macro (hardened against format string attacks) ─── */
#define panic(fmt, ...)                                                          \
    do {                                                                         \
        fprintf(stderr, "PANIC [%s:%d]: " fmt "\n", __FILENAME__, __LINE__,     \
                ##__VA_ARGS__);                                                  \
        exit(EXIT_FAILURE);                                                      \
    } while (0)

/* Safe panic for raw string messages (no format interpretation) */
#define panic_msg(msg) panic("%s", (msg))

/* ─── Extension stripper (single-evaluation, safe) ─── */
#define extension_remover(filename, output_buf, buf_size)                        \
    do {                                                                         \
        const char* _feature_filename = (filename);                              \
        char* _feature_output = (output_buf);                                    \
        size_t _feature_buf_size = (buf_size);                                   \
                                                                                 \
        if (_feature_filename == NULL) {                                         \
            panic_msg("extension_remover received a NULL filename");             \
        }                                                                        \
        if (_feature_output == NULL) {                                           \
            panic_msg("extension_remover received a NULL output buffer");        \
        }                                                                        \
        if (_feature_buf_size == 0) {                                            \
            panic_msg("extension_remover received a zero-size buffer");          \
        }                                                                        \
        const char* _feature_dot = strrchr(_feature_filename, '.');              \
        size_t _feature_len = (_feature_dot != NULL)                             \
                              ? (size_t)(_feature_dot - _feature_filename)       \
                              : strlen(_feature_filename);                       \
        if (_feature_len >= _feature_buf_size) {                                 \
            panic("Output buffer too small: need %zu, have %zu",                 \
                  _feature_len + 1, _feature_buf_size);                          \
        }                                                                        \
        memcpy(_feature_output, _feature_filename, _feature_len);                \
        _feature_output[_feature_len] = '\0';                                   \
    } while (0)

/* ─── CPU time tracer ─── */
#define TraceTime(block_name, block_code)                                        \
    do {                                                                         \
        clock_t _feature_start = clock();                                        \
        block_code;                                                              \
        clock_t _feature_end = clock();                                          \
        double _feature_cpu_time = ((double)(_feature_end - _feature_start))     \
                                   / CLOCKS_PER_SEC;                             \
        fprintf(stdout, "[PERF]: '%s' took %.6f seconds (CPU)\n",                \
                (block_name), _feature_cpu_time);                                \
    } while (0)

/* ─── Wall-clock time tracer ─── */
#if FEATURE_PLATFORM_WINDOWS

/* Thread-safe QPC frequency initialization using InterlockedExchange */
static inline LARGE_INTEGER _feature_qpc_frequency(void) {
    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq)) {
        panic_msg("QueryPerformanceFrequency failed");
    }
    return freq;
}

static inline LARGE_INTEGER _feature_get_qpc_freq(void) {
    static LARGE_INTEGER freq = {0};
    static volatile LONG initialized = 0;

    if (initialized == 0) {
        LARGE_INTEGER f = _feature_qpc_frequency();
        freq.QuadPart = f.QuadPart;
        _InterlockedExchange(&initialized, 1);
    }
    return freq;
}

#define TraceTimeWall(block_name, block_code)                                    \
    do {                                                                         \
        LARGE_INTEGER _feature_freq = _feature_get_qpc_freq();                   \
        LARGE_INTEGER _feature_start, _feature_end;                              \
        QueryPerformanceCounter(&_feature_start);                                \
        block_code;                                                              \
        QueryPerformanceCounter(&_feature_end);                                  \
        double _feature_elapsed = (double)(_feature_end.QuadPart                 \
                                            - _feature_start.QuadPart)           \
                                  / _feature_freq.QuadPart;                      \
        fprintf(stdout, "[PERF]: '%s' took %.6f seconds (wall)\n",              \
                (block_name), _feature_elapsed);                                 \
    } while (0)

/* ─── High-res CPU tracer (Windows thread time) ─── */
#define TraceTimeThread(block_name, block_code)                                  \
    do {                                                                         \
        FILETIME _feature_ft_create, _feature_ft_exit;                           \
        FILETIME _feature_ft_kernel_start, _feature_ft_kernel_end;               \
        FILETIME _feature_ft_user_start, _feature_ft_user_end;                   \
        ULARGE_INTEGER _feature_kernel_start, _feature_kernel_end;               \
        ULARGE_INTEGER _feature_user_start, _feature_user_end;                   \
                                                                                 \
        GetThreadTimes(GetCurrentThread(), &_feature_ft_create,                  \
                       &_feature_ft_exit, &_feature_ft_kernel_start,              \
                       &_feature_ft_user_start);                                 \
        block_code;                                                              \
        GetThreadTimes(GetCurrentThread(), &_feature_ft_create,                  \
                       &_feature_ft_exit, &_feature_ft_kernel_end,               \
                       &_feature_ft_user_end);                                   \
                                                                                 \
        _feature_kernel_start.LowPart = _feature_ft_kernel_start.dwLowDateTime;  \
        _feature_kernel_start.HighPart = _feature_ft_kernel_start.dwHighDateTime;\
        _feature_kernel_end.LowPart = _feature_ft_kernel_end.dwLowDateTime;      \
        _feature_kernel_end.HighPart = _feature_ft_kernel_end.dwHighDateTime;    \
        double _feature_kernel = (double)(_feature_kernel_end.QuadPart           \
                                          - _feature_kernel_start.QuadPart)      \
                                 / 1e7;                                          \
                                                                                 \
        _feature_user_start.LowPart = _feature_ft_user_start.dwLowDateTime;      \
        _feature_user_start.HighPart = _feature_ft_user_start.dwHighDateTime;    \
        _feature_user_end.LowPart = _feature_ft_user_end.dwLowDateTime;          \
        _feature_user_end.HighPart = _feature_ft_user_end.dwHighDateTime;        \
        double _feature_user = (double)(_feature_user_end.QuadPart               \
                                        - _feature_user_start.QuadPart)          \
                               / 1e7;                                            \
                                                                                 \
        fprintf(stdout, "[PERF]: '%s' took %.6f seconds (kernel: %.6f, user: %.6f)\n",\
                (block_name), _feature_kernel + _feature_user,                   \
                _feature_kernel, _feature_user);                                 \
    } while (0)

#elif FEATURE_PLATFORM_MACOS

/* macOS: Use mach_absolute_time for high-resolution monotonic timing
 * clock_gettime is only available on macOS 10.12+; mach_absolute_time
 * has been available since OS X 10.0 and provides nanosecond resolution.
 */
static inline double _feature_mach_seconds_per_tick(void) {
    static double seconds_per_tick = 0.0;
    static volatile int initialized = 0;

    if (initialized == 0) {
        mach_timebase_info_data_t timebase;
        if (mach_timebase_info(&timebase) != KERN_SUCCESS) {
            panic_msg("mach_timebase_info failed");
        }
        seconds_per_tick = (double)timebase.numer / (double)timebase.denom / 1e9;
        __sync_synchronize();  /* Memory barrier */
        initialized = 1;
    }
    return seconds_per_tick;
}

#define TraceTimeWall(block_name, block_code)                                    \
    do {                                                                         \
        double _feature_spt = _feature_mach_seconds_per_tick();                \
        uint64_t _feature_start = mach_absolute_time();                          \
        block_code;                                                              \
        uint64_t _feature_end = mach_absolute_time();                          \
        double _feature_elapsed = (double)(_feature_end - _feature_start)      \
                                  * _feature_spt;                                \
        fprintf(stdout, "[PERF]: '%s' took %.6f seconds (wall, mach)\n",        \
                (block_name), _feature_elapsed);                                 \
    } while (0)

/* Thread time not available on macOS — fallback to CPU time */
#define TraceTimeThread(block_name, block_code)                                  \
    do {                                                                         \
        fprintf(stdout, "[PERF]: Thread time unavailable on this platform, "     \
                "falling back to CPU time for '%s'\n", (block_name));            \
        TraceTime((block_name), (block_code));                                   \
    } while (0)

#else  /* POSIX Linux/BSD */

/* POSIX: Use clock_gettime with CLOCK_MONOTONIC for monotonic wall time.
 * Available since glibc 2.17 (no -lrt needed), POSIX.1-2008 mandatory.
 * For older systems, we fall back to gettimeofday with a deprecation warning.
 */

/* Check if clock_gettime is available at compile time */
#if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0)
  #define FEATURE_HAS_CLOCK_GETTIME 1
#else
  #define FEATURE_HAS_CLOCK_GETTIME 0
#endif

#if FEATURE_HAS_CLOCK_GETTIME
#define TraceTimeWall(block_name, block_code)                                    \
    do {                                                                         \
        struct timespec _feature_start, _feature_end;                            \
        if (clock_gettime(CLOCK_MONOTONIC, &_feature_start) != 0) {              \
            panic_msg("clock_gettime(CLOCK_MONOTONIC) failed");                  \
        }                                                                        \
        block_code;                                                              \
        if (clock_gettime(CLOCK_MONOTONIC, &_feature_end) != 0) {              \
            panic_msg("clock_gettime(CLOCK_MONOTONIC) failed");                  \
        }                                                                        \
        double _feature_elapsed = (_feature_end.tv_sec - _feature_start.tv_sec)  \
                                  + (_feature_end.tv_nsec - _feature_start.tv_nsec)\
                                    / 1e9;                                       \
        fprintf(stdout, "[PERF]: '%s' took %.6f seconds (wall)\n",              \
                (block_name), _feature_elapsed);                                 \
    } while (0)
#else
/* Fallback for very old systems without clock_gettime */
#define TraceTimeWall(block_name, block_code)                                    \
    do {                                                                         \
        struct timeval _feature_start, _feature_end;                             \
        if (gettimeofday(&_feature_start, NULL) != 0) {                          \
            panic_msg("gettimeofday failed");                                    \
        }                                                                        \
        block_code;                                                              \
        if (gettimeofday(&_feature_end, NULL) != 0) {                            \
            panic_msg("gettimeofday failed");                                    \
        }                                                                        \
        double _feature_elapsed = (_feature_end.tv_sec - _feature_start.tv_sec)  \
                                  + (_feature_end.tv_usec - _feature_start.tv_usec)\
                                    / 1e6;                                       \
        fprintf(stdout, "[PERF]: '%s' took %.6f seconds (wall, deprecated gettimeofday)\n",\
                (block_name), _feature_elapsed);                                 \
    } while (0)
#endif

/* Thread time: Use CLOCK_THREAD_CPUTIME_ID if available */
#if defined(CLOCK_THREAD_CPUTIME_ID)
#define TraceTimeThread(block_name, block_code)                                  \
    do {                                                                         \
        struct timespec _feature_start, _feature_end;                            \
        if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &_feature_start) != 0) {    \
            fprintf(stdout, "[PERF]: Thread time unavailable, falling back to CPU time for '%s'\n",\
                    (block_name));                                               \
            TraceTime((block_name), (block_code));                              \
        } else {                                                                 \
            block_code;                                                          \
            if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &_feature_end) != 0) {    \
                panic_msg("clock_gettime(CLOCK_THREAD_CPUTIME_ID) failed");        \
            }                                                                    \
            double _feature_elapsed = (_feature_end.tv_sec - _feature_start.tv_sec)\
                                      + (_feature_end.tv_nsec - _feature_start.tv_nsec)\
                                        / 1e9;                                   \
            fprintf(stdout, "[PERF]: '%s' took %.6f seconds (thread CPU)\n",     \
                    (block_name), _feature_elapsed);                             \
        }                                                                        \
    } while (0)
#else
#define TraceTimeThread(block_name, block_code)                                  \
    do {                                                                         \
        fprintf(stdout, "[PERF]: Thread time unavailable on this platform, "     \
                "falling back to CPU time for '%s'\n", (block_name));            \
        TraceTime((block_name), (block_code));                                   \
    } while (0)
#endif

#endif  /* FEATURE_PLATFORM_WINDOWS / FEATURE_PLATFORM_MACOS / POSIX */

/* ─── Safe memory allocation ─── */

#if FEATURE_HAS_STATEMENT_EXPRESSION

/* GNU C statement expression version (GCC, Clang) */
#define safe_malloc(size)                                                        \
    ({                                                                           \
        size_t _feature_size = (size_t)(size);                                   \
        void* _feature_ptr = malloc(_feature_size);                              \
        if (_feature_ptr == NULL) {                                              \
            panic("malloc(%zu) failed", _feature_size);                          \
        }                                                                        \
        _feature_ptr;                                                            \
    })

#define safe_calloc(count, size)                                                 \
    ({                                                                           \
        size_t _feature_count = (size_t)(count);                                 \
        size_t _feature_size = (size_t)(size);                                   \
        void* _feature_ptr = calloc(_feature_count, _feature_size);              \
        if (_feature_ptr == NULL) {                                              \
            panic("calloc(%zu, %zu) failed", _feature_count, _feature_size);     \
        }                                                                        \
        _feature_ptr;                                                            \
    })

/*
 * FIXED safe_realloc:
 * - Does NOT free old pointer on failure (original had double-free bug)
 * - Old pointer remains valid on failure; caller can retry or handle error
 * - Only panics; if you need graceful failure handling, use realloc directly
 */
#define safe_realloc(ptr, size)                                                  \
    ({                                                                           \
        void* _feature_old_ptr = (void*)(ptr);                                   \
        size_t _feature_size = (size_t)(size);                                   \
        void* _feature_ptr = realloc(_feature_old_ptr, _feature_size);           \
        if (_feature_ptr == NULL && _feature_size > 0) {                         \
            /* realloc failed but old pointer is still valid — DO NOT free it */ \
            panic("realloc(%p, %zu) failed (original pointer still valid)",      \
                  _feature_old_ptr, _feature_size);                              \
        }                                                                        \
        _feature_ptr;                                                            \
    })

#else  /* MSVC or other compilers without statement expressions */

/*
 * MSVC-compatible inline function versions.
 * Usage: ptr = safe_malloc(size); (same syntax as GNU version)
 */
static inline void* safe_malloc_impl(size_t size, const char* file, int line) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "PANIC [%s:%d]: malloc(%zu) failed\n",
                _feature_basename(file), line, size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

static inline void* safe_calloc_impl(size_t count, size_t size,
                                      const char* file, int line) {
    void* ptr = calloc(count, size);
    if (ptr == NULL) {
        fprintf(stderr, "PANIC [%s:%d]: calloc(%zu, %zu) failed\n",
                _feature_basename(file), line, count, size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

/* FIXED: safe_realloc_impl — does not free old pointer on failure */
static inline void* safe_realloc_impl(void* ptr, size_t size,
                                       const char* file, int line) {
    void* new_ptr = realloc(ptr, size);
    if (new_ptr == NULL && size > 0) {
        fprintf(stderr, "PANIC [%s:%d]: realloc(%p, %zu) failed "
                "(original pointer still valid)\n",
                _feature_basename(file), line, ptr, size);
        exit(EXIT_FAILURE);
    }
    return new_ptr;
}

/* MSVC macros — same syntax as GNU versions */
#define safe_malloc(size) safe_malloc_impl((size_t)(size), __FILE__, __LINE__)
#define safe_calloc(count, size) safe_calloc_impl((size_t)(count), (size_t)(size), __FILE__, __LINE__)
#define safe_realloc(ptr, size) safe_realloc_impl((void*)(ptr), (size_t)(size), __FILE__, __LINE__)

#endif  /* FEATURE_HAS_STATEMENT_EXPRESSION */

/* ─── Safe string copy (single-evaluation, null-terminated) ─── */
#define str_copy(dest, src, dest_size)                                           \
    do {                                                                         \
        char* _feature_dest = (dest);                                            \
        const char* _feature_src = (src);                                        \
        size_t _feature_dest_size = (dest_size);                                 \
                                                                                 \
        if (_feature_dest == NULL || _feature_src == NULL) {                     \
            panic_msg("str_copy received NULL pointer");                         \
        }                                                                        \
        if (_feature_dest_size == 0) {                                           \
            panic_msg("str_copy received zero-size destination buffer");         \
        }                                                                        \
        size_t _feature_src_len = strlen(_feature_src);                          \
        if (_feature_src_len >= _feature_dest_size) {                            \
            panic("str_copy buffer too small: need %zu, have %zu",               \
                  _feature_src_len + 1, _feature_dest_size);                     \
        }                                                                        \
        memcpy(_feature_dest, _feature_src, _feature_src_len);                   \
        _feature_dest[_feature_src_len] = '\0';                                 \
    } while (0)

/* ─── Scoped cleanup (GNU cleanup attribute) ─── */
#if FEATURE_HAS_CLEANUP_ATTRIBUTE

#define defer_free(ptr)                                                          \
    void* _FEATURE_UNIQUE(_feature_cleanup_)                                     \
    __attribute__((cleanup(_feature_auto_free))) = (ptr)

/* The cleanup handler receives a pointer to the variable */
static inline void _feature_auto_free(void* p) {
    void** pp = (void**)p;
    if (*pp != NULL) {
        free(*pp);
        *pp = NULL;
    }
}

#else  /* MSVC or other compilers without cleanup attribute */

/*
 * IMPROVED MSVC fallback: Manual cleanup tracking with a per-function stack.
 * Supports up to 16 defer_free calls per function.
 *
 * Usage:
 *   defer_free(ptr1);
 *   defer_free(ptr2);
 *   // ... use pointers ...
 *   defer_cleanup();  // Frees all tracked pointers (call before return)
 */
#if FEATURE_PLATFORM_WINDOWS

#define _FEATURE_DEFER_MAX 16

__declspec(thread) static void* _feature_defer_stack[_FEATURE_DEFER_MAX] = {0};
__declspec(thread) static int _feature_defer_count = 0;

#define defer_free(ptr)                                                          \
    do {                                                                         \
        void* _feature_p = (ptr);                                                \
        if (_feature_defer_count < _FEATURE_DEFER_MAX) {                         \
            _feature_defer_stack[_feature_defer_count++] = _feature_p;           \
        } else {                                                                 \
            fprintf(stderr, "WARNING: defer_free stack overflow at %s:%d\n",     \
                    __FILENAME__, __LINE__);                                     \
        }                                                                        \
    } while (0)

#define defer_cleanup()                                                          \
    do {                                                                         \
        while (_feature_defer_count > 0) {                                       \
            void* _feature_p = _feature_defer_stack[--_feature_defer_count];   \
            if (_feature_p != NULL) {                                            \
                free(_feature_p);                                                \
            }                                                                    \
        }                                                                        \
    } while (0)

#else  /* Non-Windows, non-GNU compiler */

/* Minimal fallback: just warn */
#define defer_free(ptr)                                                          \
    do {                                                                         \
        (void)(ptr);  /* Silence unused-variable warnings */                      \
        _Pragma("message(\"WARNING: defer_free is a no-op on this compiler. "    \
                "Manual cleanup required.\")");                                  \
    } while (0)

#define defer_cleanup() ((void)0)

#endif  /* FEATURE_PLATFORM_WINDOWS */

#endif  /* FEATURE_HAS_CLEANUP_ATTRIBUTE */

#ifdef __cplusplus
}
#endif

#endif  /* FEATURE_H */
