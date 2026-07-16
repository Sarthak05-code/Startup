#include "Feature.h"
#include <stdio.h>
void caller(void);

void timeRunner(void) {
  char clean_name[256];
  TraceTime(
      "Strip Extension loop: ", for (int i = 0; i < 1000000; ++i) {
        extension_remover("my_test_file.png.img", clean_name,
                          sizeof(clean_name));
      });
}

int main(void) {
  timeRunner();
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
  Panic("The function did not run further");

  while (1) {
    if (filename == NULL)
      goto up;
    else
      printf("The file name is : %s", filename);
  }
}
