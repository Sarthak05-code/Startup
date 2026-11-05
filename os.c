
#include <stdbool.h>
#include <stdio.h>


#define P 5
#define R 3

int main() {

  // Currently allocated resources
  int alloc[P][R] = {{0, 1, 0}, {2, 0, 0}, {3, 0, 3}, {2, 1, 1}, {0, 0, 2}};

  // Remaining resource requests
  int request[P][R] = {{0, 0, 0}, {2, 0, 2}, {0, 0, 0}, {1, 0, 0}, {0, 0, 2}};

  // Currently available resources
  int avail[R] = {0, 0, 0};

  int work[R];
  bool finish[P] = {false};

  // Step 1: Initialize Work = Available
  for (int j = 0; j < R; j++) {
    work[j] = avail[j];
  }

  // Step 2: Mark processes holding no resources as finished
  for (int i = 0; i < P; i++) {
    bool holding_resources = false;

    for (int j = 0; j < R; j++) {
      if (alloc[i][j] != 0) {
        holding_resources = true;
        break;
      }
    }

    if (!holding_resources) {
      finish[i] = true;
    }
  }

  // Step 3: Deadlock Detection
  bool found;

  do {
    found = false;

    for (int i = 0; i < P; i++) {

      if (!finish[i]) {

        bool can_finish = true;

        // Check Request <= Work
        for (int j = 0; j < R; j++) {
          if (request[i][j] > work[j]) {
            can_finish = false;
            break;
          }
        }

        // If process can finish, release its resources
        if (can_finish) {

          for (int j = 0; j < R; j++) {
            work[j] += alloc[i][j];
          }

          finish[i] = true;
          found = true;
        }
      }
    }

  } while (found);

  // Step 4: Check for deadlocked processes
  bool deadlock = false;

  printf("Deadlocked processes: ");

  for (int i = 0; i < P; i++) {
    if (!finish[i]) {
      printf("P%d ", i);
      deadlock = true;
    }
  }

  if (deadlock) {
    printf("\nSYSTEM IS IN DEADLOCK\n");
  } else {
    printf("None\nSYSTEM IS NOT IN DEADLOCK\n");
  }

  return 0;
}
