#include <synchapi.h>
#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <windows.h>

int main(void) {
  printf("Starting the program...");

  Sleep(1000);

  printf("Displaying the message below.\n");

  int choice = MessageBoxA(NULL, "Hello! this is native windows api pop-up.\n",
                           "Windows API DEMO", MB_YESNO | MB_ICONINFORMATION);

  if (choice == IDYES) {
    printf("The user clicked `YES`.\n");
  } else if (choice == IDNO) {
    printf("The user clicked `NO`.\n");
  }
  return 0;
}
