#include <stdio.h>
#include <stdlib.h>

int main() {
  int n, head, total = 0;
  int disk_size = 200;

  printf("Enter number of requests: ");
  scanf("%d", &n);

  int request[n];

  printf("Enter request queue:\n");
  for (int i = 0; i < n; i++) {
    scanf("%d", &request[i]);
  }

  printf("Enter initial head position: ");
  scanf("%d", &head);

  // Sort requests in ascending order
  for (int i = 0; i < n - 1; i++) {
    for (int j = 0; j < n - i - 1; j++) {
      if (request[j] > request[j + 1]) {
        int temp = request[j];
        request[j] = request[j + 1];
        request[j + 1] = temp;
      }
    }
  }

  int i = 0;

  // Find first request greater than or equal to head
  while (i < n && request[i] < head) {
    i++;
  }

  printf("\nSeek Sequence: %d", head);

  // Move towards higher cylinders
  for (int j = i; j < n; j++) {
    total += abs(head - request[j]);
    head = request[j];
    printf(" -> %d", head);
  }

  // Reach the last cylinder
  total += abs(head - (disk_size - 1));
  head = disk_size - 1;
  printf(" -> %d", head);

  // Jump to cylinder 0
  total += disk_size - 1;
  head = 0;
  printf(" -> %d", head);

  // Continue towards higher cylinders
  for (int j = 0; j < i; j++) {
    total += abs(head - request[j]);
    head = request[j];
    printf(" -> %d", head);
  }

  printf("\nTotal Head Movement: %d\n", total);

  return 0;
}
