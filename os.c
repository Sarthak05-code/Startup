

#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <windows.h>

int main(void) {
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
  printf("This is bright red text.\n");

  SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
  printf("This is bright green text.\n");

  SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_RED |
                                        FOREGROUND_INTENSITY);
  printf("This is bright purple text.\n");

  SetConsoleTextAttribute(hConsole,
                          FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
  printf("Back to normal.\n");

  return 0;
}
