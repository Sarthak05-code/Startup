#include "Feature.h"
#include <stdio.h>

int main(void) {
  char buffer[256];

  extension_remover("document.pdf", buffer, sizeof(buffer));

  printf("Stripped: %s\n", buffer);

  int *array = safe_malloc(100 * sizeof(int));
  defer_free(array);

  for (int i = 0; i < 100; ++i) {
    array[i] = i;
  }

  TraceTimeWall("Loop Test", {
    for (volatile int i = 0; i < 100000; ++i)
      ;
  });

  return 0;
}
