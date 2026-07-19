#include "Feature.h"
#include <stdio.h>

volatile int number = 10;

void gotoTester(void) {
error:
  panic("This runs when we find an error: \n");

  if (number == 10) {
    goto error;
  }
  printf("This won't run if the value is 10: \n");
}

void anotherGotoTester(void) {
  (void)printf("Typecasting a printf with void... will there be any change?\n");
}

int main(void) {
  printf("This is the program\n");
  gotoTester();
}