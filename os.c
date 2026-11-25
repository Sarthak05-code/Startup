#include <stdio.h>

#define BASE_SPEED 107.0
#define SPEED_THRESHOLD 140.0
#define BASE_ELATION_PERCENTAGE 0.0
#define ELATION_PERCENTAGE 40.0
#define ELATION_PERCENTAGE 40.0
#define MAX_ELATION_PERCENTAGE 100.0

#define MAX_SPEED_TO_ELATION_LIMIT (MAX_ELATION_PERCENTAGE - ELATION_PERCENTAGE)

typedef struct {
  double speed;
  double elation_percentage;
} Stats;

int main(void) {
  Stats stats;

  stats.speed = BASE_SPEED;
  stats.elation_percentage = BASE_ELATION_PERCENTAGE;

  stats.speed += 150; // 107 + 150 = 257 -> despite a lot of speed , the code
                      // won't overcap elation

  if (stats.speed == SPEED_THRESHOLD) {
    stats.elation_percentage = ELATION_PERCENTAGE;
  } else if (stats.speed > SPEED_THRESHOLD) {
    double extra_bonus = stats.speed - SPEED_THRESHOLD;

    if (extra_bonus > MAX_SPEED_TO_ELATION_LIMIT) {
      stats.elation_percentage = 100.0;
    } else {

      stats.elation_percentage = ELATION_PERCENTAGE + extra_bonus;
    }
  }

  printf("Final speed: %.2lf\n", stats.speed);
  printf("Final elation: %.2lf%%\n", stats.elation_percentage);

  return 0;
}
