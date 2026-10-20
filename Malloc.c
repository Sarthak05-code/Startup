#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { LOW, MEDIUM, HIGH } State;

typedef struct {
  int level_id;
  char level_name[20];
  State priority;
} Level; // plain struct, so we can keep an array of them

const char *state_to_str(State s) {
  switch (s) {
  case LOW:
    return "LOW";
  case MEDIUM:
    return "MEDIUM";
  case HIGH:
    return "HIGH";
  default:
    return "UNKNOWN";
  }
}

static int max_int(int a, int b) { return a > b ? a : b; }

static int digits(int n) { return snprintf(NULL, 0, "%d", n); }

static void print_border(int w1, int w2, int w3) {
  int w[3] = {w1, w2, w3};
  for (int c = 0; c < 3; c++) {
    putchar('+');
    for (int i = 0; i < w[c] + 2; i++)
      putchar('-');
  }
  puts("+");
}

static void print_table(const Level *rows, int count) {
  // Steps 2-3: start from header widths, widen for every row
  int w1 = (int)strlen("ID");
  int w2 = (int)strlen("Name");
  int w3 = (int)strlen("Priority");

  for (int i = 0; i < count; i++) {
    w1 = max_int(w1, digits(rows[i].level_id));
    w2 = max_int(w2, (int)strlen(rows[i].level_name));
    w3 = max_int(w3, (int)strlen(state_to_str(rows[i].priority)));
  }

  // Steps 4-5: draw
  print_border(w1, w2, w3);
  printf("| %-*s | %-*s | %-*s |\n", w1, "ID", w2, "Name", w3, "Priority");
  print_border(w1, w2, w3);
  for (int i = 0; i < count; i++) {
    printf("| %-*d | %-*s | %-*s |\n", w1, rows[i].level_id, w2,
           rows[i].level_name, w3, state_to_str(rows[i].priority));
  }
  print_border(w1, w2, w3);
}

int main(void) {
  Level *rows = NULL;
  int count = 0, cap = 0;
  char buf[64];

  for (;;) {
    printf("ID (q to quit): ");
    if (!fgets(buf, sizeof buf, stdin) || buf[0] == 'q')
      break;
    int id = atoi(buf);

    printf("Name: ");
    if (!fgets(buf, sizeof buf, stdin))
      break;
    buf[strcspn(buf, "\n")] = '\0'; // strip newline

    char name[64];
    snprintf(name, sizeof name, "%s", buf);

    printf("Priority (0=LOW, 1=MEDIUM, 2=HIGH): ");
    if (!fgets(buf, sizeof buf, stdin))
      break;
    int p = atoi(buf);
    if (p < LOW || p > HIGH)
      p = LOW;

    if (count == cap) { // grow the array when full
      cap = cap ? cap * 2 : 4;
      Level *tmp = realloc(rows, cap * sizeof *rows);
      if (!tmp) {
        free(rows);
        return 1;
      }
      rows = tmp;
    }

    rows[count].level_id = id;
    snprintf(rows[count].level_name, sizeof rows[count].level_name, "%s", name);
    rows[count].priority = (State)p;
    count++;

    print_table(rows, count); // border re-fits every time
  }

  free(rows);
  return 0;
}
