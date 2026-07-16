#include "Feature.h"
#include <stdio.h>
void caller(void);

int main(void) {
  const char *c = "family.txt";

  if (c == NULL) {
    Panic("name was found to be null");
  } else {
    printf("The file name was : %s\n", c);
  }

  char cleanbuffer[256];
  extension_remover(c, cleanbuffer, sizeof(cleanbuffer));
  printf("The name without the extension is: %s\n", cleanbuffer);

  caller();

  return 0;
}

void caller(void) {

  const char *filename = NULL;
up:
  Panic("The function did run");

  while (1) {
    if (filename == NULL)
      goto up;
    else
      printf("The file name is : %s", filename);
  }
}
