#include "Feature.h"
#include <stdio.h>

// 1. Declare the prototype so recursionTester knows 'rec' exists
int rec(int n);
void caller(void);
void recursionTester(void);
void timeRunner(void);

void timeRunner(void) {
  char clean_name[256];
  TraceTime(
      "Strip Extension loop: ", for (int i = 0; i < 1000000; ++i) {
        extension_remover("my_test_file.png.img", clean_name,
                          sizeof(clean_name));
      });
}

// 2. Update recursionTester to call and time 'rec'
void recursionTester(void) {
  int num =
      30; // Bumped up slightly to 30 so it takes a measurable amount of time!
  volatile int
      result; // 'volatile' ensures the compiler doesn't optimize the call away

  TraceTime("Recursion calculation", result = rec(num));

  printf("Result of Fibonacci(%d) = %d\n", num, result);
}

int rec(int n) {
  if (n <= 1) // 0 and 1
    return n;
  return rec(n - 2) + rec(n - 1);
}

int main(void) {
  timeRunner();

  // 3. Call your new tester in main
  printf("\n--- Running Recursion Test ---\n");
  recursionTester();
  printf("-----------------------------\n\n");

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
    if (filename == novalue)
      goto up;
    else
      printf("The file name is : %s", filename);
  }
}
