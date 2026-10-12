#include "Feature.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// FIX: Added 'size_t len' as a parameter
void printer(const int arr[], size_t len) {
  for (size_t i = 0; i < len;
       ++i) { // Using size_t for the loop index to match len
    printf("%d\t", arr[i]);
  }
  printf("\n"); // Added a newline for cleaner output formatting
}

// NOTE : Main point of the code.

int main(void) {
  int n;
  printf("Enter the number of data: "); // Cleaned up prompt spacing
  if (scanf("%d", &n) != 1 || n <= 0) { // Good practice: validate user input
    printf("Invalid input size.\n");
    return 1;
  }

  int *storage = malloc(n * sizeof(int));

  // Typo fixed: "No memory allocated"
  if (storage == NULL) {
    printf("No memory allocated.\n");
    return 1;
  }

  for (int i = 0; i < n; ++i) {
    printf("Enter the no %d: ", i + 1); // Cleaned up prompt spacing
    scanf("%d", &storage[i]);
  }

  // FIX: Passed 'n' explicitly into the printer function
  printer(storage, n);

  free(storage);
  storage = NULL;
  return 0;
}
