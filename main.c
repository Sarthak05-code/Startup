#include "Feature.h"
#include <stdio.h>

typedef struct {
  int singleThread;
  long long longerThread;
} Process_type;

int main(void) {
  Process_type process = {0};
 
  printf("The single and multiple thread = %d %lld\n ", process.singleThread,
         process.longerThread);
  return 0;
}
