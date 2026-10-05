#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 8
#define MAX_LOAD_FACTOR 0.75

typedef struct Node {
  char *key;
  int value;
  struct Node *next;
} Node;

typedef struct {
  Node **buckets;  // array of Node* (each one is the head of a chain)
  size_t capacity; // number of buckets
  size_t size;     // number of key/value pairs stored
} HashMap;

/* Portable replacement for strdup (which isn't standard C before C23). */
static char *copy_string(const char *src) {
  size_t len = strlen(src) + 1;
  char *dst = malloc(len);
  if (dst == NULL) {
    return NULL;
  }
  memcpy(dst, src, len);
  return dst;
}

/* djb2 hash. Returns the full hash; the caller applies % capacity,
   because capacity changes when the map grows. */
static size_t hash_function(const char *str) {
  size_t hash = 5381;
  const unsigned char *p = (const unsigned char *)str;
  while (*p) {
    hash = ((hash << 5) + hash) + *p;
    p++;
  }
  return hash;
}

HashMap *map_create(void) {
  HashMap *map = malloc(sizeof(HashMap));
  if (map == NULL) {
    return NULL;
  }
  map->buckets = calloc(INITIAL_CAPACITY, sizeof(Node *)); // zeroed = all NULL
  if (map->buckets == NULL) {
    free(map);
    return NULL;
  }
  map->capacity = INITIAL_CAPACITY;
  map->size = 0;
  return map;
}

/* Double the number of buckets and move every node to its new bucket.
   Nodes are re-linked, not copied, so no extra allocation per node. */
static bool map_resize(HashMap *map) {
  size_t new_capacity = map->capacity * 2;
  Node **new_buckets = calloc(new_capacity, sizeof(Node *));
  if (new_buckets == NULL) {
    return false; // keep using the old table
  }

  for (size_t i = 0; i < map->capacity; ++i) {
    Node *node = map->buckets[i];
    while (node != NULL) {
      Node *next = node->next; // save before we overwrite node->next
      size_t index = hash_function(node->key) % new_capacity;
      node->next = new_buckets[index];
      new_buckets[index] = node;
      node = next;
    }
  }

  free(map->buckets);
  map->buckets = new_buckets;
  map->capacity = new_capacity;
  return true;
}

/* Returns true on success, false if memory ran out. */
bool map_put(HashMap *map, const char *key, int value) {
  size_t index = hash_function(key) % map->capacity;

  // 1. Key already exists? Update it.
  for (Node *node = map->buckets[index]; node != NULL; node = node->next) {
    if (strcmp(node->key, key) == 0) {
      node->value = value;
      return true;
    }
  }

  // 2. Too full? Grow first, then recompute the index.
  if ((double)(map->size + 1) / (double)map->capacity > MAX_LOAD_FACTOR) {
    map_resize(map); // if this fails we just carry on with longer chains
    index = hash_function(key) % map->capacity;
  }

  // 3. Insert a new node at the head of the chain.
  Node *new_node = malloc(sizeof(Node));
  if (new_node == NULL) {
    return false;
  }
  new_node->key = copy_string(key);
  if (new_node->key == NULL) {
    free(new_node);
    return false;
  }
  new_node->value = value;
  new_node->next = map->buckets[index];
  map->buckets[index] = new_node;
  map->size++;
  return true;
}

/* Returns true if found and writes the value into *out_value.
   This avoids the "what if -1 is a real value?" problem of default values. */
bool map_get(const HashMap *map, const char *key, int *out_value) {
  size_t index = hash_function(key) % map->capacity;
  for (Node *node = map->buckets[index]; node != NULL; node = node->next) {
    if (strcmp(node->key, key) == 0) {
      *out_value = node->value;
      return true;
    }
  }
  return false;
}

/* Returns true if the key existed and was removed. */
bool map_remove(HashMap *map, const char *key) {
  size_t index = hash_function(key) % map->capacity;
  Node *prev = NULL;
  Node *node = map->buckets[index];

  while (node != NULL) {
    if (strcmp(node->key, key) == 0) {
      if (prev == NULL) {
        map->buckets[index] = node->next; // removing the head
      } else {
        prev->next = node->next; // removing from the middle/end
      }
      free(node->key);
      free(node);
      map->size--;
      return true;
    }
    prev = node;
    node = node->next;
  }
  return false;
}

void map_free(HashMap *map) {
  if (map == NULL) {
    return;
  }
  for (size_t i = 0; i < map->capacity; ++i) {
    Node *node = map->buckets[i];
    while (node != NULL) {
      Node *next = node->next;
      free(node->key);
      free(node);
      node = next;
    }
  }
  free(map->buckets);
  free(map);
}

static void print_lookup(const HashMap *map, const char *key) {
  int value;
  if (map_get(map, key, &value)) {
    printf("%-8s -> %d\n", key, value);
  } else {
    printf("%-8s -> not found\n", key);
  }
}

void map_print(const HashMap *map) {
  printf("HashMap : size = %zu | capacity : %zu\n", map->size, map->capacity);

  for (size_t i = 0; i < map->capacity; ++i) {
    printf("[%2zu]", i);
    for (const Node *node = map->buckets[i]; node != NULL; node = node->next) {
      printf("(%s | %d) > ", node->key, node->value);
    }
    printf("Null\n");
  }
}

int main(void) {
  HashMap *map = map_create();
  if (map == NULL) {
    fprintf(stderr, "Out of memory\n");
    return 1;
  }

  map_put(map, "Sarthak", 25);
  map_put(map, "Sagar", 10);
  map_put(map, "Ayush", 12);
  map_put(map, "Sarthak", 26); // update, size stays the same

  print_lookup(map, "Sarthak");
  print_lookup(map, "Sagar");
  print_lookup(map, "Ayush");
  print_lookup(map, "Nobody");

  printf("\nsize=%zu capacity=%zu\n", map->size, map->capacity);

  // Add more keys to trigger a resize
  const char *extra[] = {"Ram", "Sita", "Hari", "Gita", "Maya", "Bikash"};
  for (size_t i = 0; i < sizeof(extra) / sizeof(extra[0]); ++i) {
    map_put(map, extra[i], (int)(i * 10));
  }
  printf("after adding more: size=%zu capacity=%zu\n", map->size,
         map->capacity);

  map_remove(map, "Sagar");
  printf("\nafter removing Sagar:\n");
  print_lookup(map, "Sagar");
  print_lookup(map, "Sarthak"); // still there after the resize
  map_print(map);

  map_free(map);
  return 0;
}
