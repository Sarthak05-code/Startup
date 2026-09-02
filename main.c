#include <stdio.h>
#define ARENA_IMPL
#include "arena.h"

int main(void) {
  Arena *a = arena_create(1024);

  arena_scope(a, {
    int *nums = arena_alloc(a, 100 * sizeof(int));
    char *msg = arena_strdup(a, "Hello from the scope.");
  });

  struct {
    char *name;
    int score;
  } *player = arena_alloc(a, sizeof(*player));
  player->name = arena_strdup(a, "Sarthak");
  player->score = 9000;
  printf("Name : %s, Score : %d \n", player->name, player->score);
  printf("Total used : %zu bytes \n", arena_total_used(a));
  arena_destroy(a);
  return 0;
}
