
#include <stdio.h>

int main() {
  int block[20], process[20];
  int nb, np, i, j;
  int start = 0, count;

  printf("Enter number of memory blocks: ");
  scanf("%d", &nb);

  printf("Enter size of memory blocks:\n");
  for (i = 0; i < nb; i++) {
    scanf("%d", &block[i]);
  }

  printf("Enter number of processes: ");
  scanf("%d", &np);

  printf("Enter size of processes:\n");
  for (i = 0; i < np; i++) {
    scanf("%d", &process[i]);
  }

  for (i = 0; i < np; i++) {
    count = 0;
    j = start;

    while (count < nb) {
      if (block[j] >= process[i]) {
        printf("Process %d allocated to Block %d\n", i + 1, j + 1);

        block[j] -= process[i];

        start = (j + 1) % nb;

        break;
      }

      j = (j + 1) % nb;
      count++;
    }

    if (count == nb) {
      printf("Process %d is not allocated\n", i + 1);
    }
  }

  return 0;
}
