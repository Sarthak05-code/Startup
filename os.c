
#include <stdio.h>
#include <stdlib.h>

int main() {
  int n, head, size;
  int request[100];
  int i, j, temp;
  int total = 0;
  int split;

  printf("Enter number of requests: ");
  scanf("%d", &n);

  printf("Enter request queue:\n");
  for (i = 0; i < n; i++) {
    scanf("%d", &request[i]);
  }

  printf("Enter initial head position: ");
  scanf("%d", &head);

  printf("Enter total number of cylinders: ");
  scanf("%d", &size);

  // Sort requests in ascending order
  for (i = 0; i < n - 1; i++) {
    for (j = 0; j < n - i - 1; j++) {
      if (request[j] > request[j + 1]) {
        temp = request[j];
        request[j] = request[j + 1];
        request[j + 1] = temp;
      }
    }
  }

  printf("\nSeek Sequence: %d", head);

  // Find the first request >= head
  split = 0;

  while (split < n && request[split] < head) {
    split++;
  }

  // Service requests in right direction
  for (i = split; i < n; i++) {
    total += abs(head - request[i]);
    head = request[i];

    printf(" -> %d", head);
  }

  // Move to the last cylinder
  if (split < n) {
    total += abs(head - (size - 1));
    head = size - 1;

    printf(" -> %d", head);
  }

  // Reverse direction and service remaining requests
  for (i = split - 1; i >= 0; i--) {
    total += abs(head - request[i]);
    head = request[i];

    printf(" -> %d", head);
  }

  printf("\nTotal Head Movement = %d\n", total);

  return 0;
}
