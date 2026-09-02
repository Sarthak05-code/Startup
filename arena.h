/* arena.h — Production arena allocator for Windows */

#ifndef ARENA_H
#define ARENA_H

#include "Feature.h"

typedef struct Arena Arena;
struct Arena {
  unsigned char *base;
  size_t used;
  size_t cap;
  Arena *next; /* overflow chain */
};

/* NOTE : Create with initial capacity. Grows automatically. */
Arena *arena_create(size_t initial_cap);

/* NOTE : Core allocation — 8-byte aligned, never returns NULL */
void *arena_alloc(Arena *a, size_t size);

/* NOTE : Zero-initialized allocation */
void *arena_calloc(Arena *a, size_t count, size_t size);

/* NOTE : Duplicate string into arena */
char *arena_strdup(Arena *a, const char *s);

/* NOTE : Duplicate memory block into arena */
void *arena_memdup(Arena *a, const void *src, size_t len);

/* NOTE : Reset all chunks to empty (memory retained, reused) */
void arena_reset(Arena *a);

/* NOTE : Total bytes currently allocated across all chunks */
size_t arena_total_used(const Arena *a);

/* NOTE : Destroy everything */
void arena_destroy(Arena *a);

/* NOTE : ─── Scoped arena (auto-reset pattern) ─── */
#define arena_scope(a, code)                                                   \
  do {                                                                         \
    size_t _arena_mark = (a)->used;                                            \
    code;                                                                      \
    (a)->used = _arena_mark;                                                   \
  } while (0)

/* NOTE : ─── Implementation ─── */
#ifdef ARENA_IMPL

#define ARENA_DEFAULT_CAP (64 * 1024)
#define ARENA_ALIGN 8

static inline size_t _arena_align_up(size_t n, size_t align) {
  return (n + align - 1) & ~(align - 1);
}

Arena *arena_create(size_t initial_cap) {
  size_t cap = initial_cap > 0 ? initial_cap : ARENA_DEFAULT_CAP;
  Arena *a = (Arena *)safe_malloc(sizeof(Arena) + cap);
  a->base = (unsigned char *)(a + 1);
  a->used = 0;
  a->cap = cap;
  a->next = NULL;
  return a;
}

void *arena_alloc(Arena *a, size_t size) {
  if (size == 0)
    return NULL;

  size_t aligned = _arena_align_up(a->used, ARENA_ALIGN);
  size_t total = aligned + size;

  if (total <= a->cap) {
    void *p = a->base + aligned;
    a->used = total;
    return p;
  }

  /* Large object (>25% of chunk): dedicated chunk */
  if (size > a->cap / 4) {
    size_t chunk_size = sizeof(Arena) + size;
    Arena *chunk = (Arena *)safe_malloc(chunk_size);
    chunk->base = (unsigned char *)(chunk + 1);
    chunk->used = size;
    chunk->cap = size;
    chunk->next = a->next;
    a->next = chunk;
    return chunk->base;
  }

  /* Grow: new chunk double the size */
  Arena *fresh = arena_create(a->cap * 2);
  fresh->next = a->next;
  a->next = fresh;
  return arena_alloc(fresh, size);
}

void *arena_calloc(Arena *a, size_t count, size_t size) {
  size_t total = count * size;
  void *p = arena_alloc(a, total);
  memset(p, 0, total);
  return p;
}

char *arena_strdup(Arena *a, const char *s) {
  if (!s)
    return NULL;
  size_t n = strlen(s) + 1;
  char *p = (char *)arena_alloc(a, n);
  memcpy(p, s, n);
  return p;
}

void *arena_memdup(Arena *a, const void *src, size_t len) {
  if (!src || len == 0)
    return NULL;
  void *p = arena_alloc(a, len);
  memcpy(p, src, len);
  return p;
}

void arena_reset(Arena *a) {
  a->used = 0;
  for (Arena *c = a->next; c != NULL; c = c->next) {
    c->used = 0;
  }
}

size_t arena_total_used(const Arena *a) {
  size_t total = a->used;
  for (Arena *c = a->next; c != NULL; c = c->next) {
    total += c->used;
  }
  return total;
}

void arena_destroy(Arena *a) {
  Arena *c = a->next;
  while (c != NULL) {
    Arena *next = c->next;
    free(c);
    c = next;
  }
  free(a);
}

#endif /* ARENA_IMPL */
#endif /* ARENA_H */
