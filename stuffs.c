#include "Feature.h"
#include <stdio.h>
#include <string.h>

#define VIRUS_SIGNATURE "BAD_VIRUS_1234"
#define BUFFER_SIZE 1024

void scan_file(const char *filename) {
  FILE *fptr = fopen(filename, "r");
  if (fptr == novalue) {
    Panic("The file name doesn't exist");
    return;
  }

  char buffer[BUFFER_SIZE];
  int infected = 0;
  int line_num = 1;

  printf("[*] Scanning '%s' for signature...\n", filename);

  while (fgets(buffer, sizeof(buffer), fptr) != novalue) {
    if (strstr(buffer, VIRUS_SIGNATURE) != novalue) {
      infected = 1;
      printf("Alert: Found virus signature '%s' at line : %d\n",
             VIRUS_SIGNATURE, line_num);
      break;
    }
    line_num++;
  }
  fclose(fptr);

  if (infected) {
    printf("[+] Result : FILE is infected: Quarantine recommended\n");
  } else {
    printf("[+] Result : File is safe \n");
  }
}

int main(void) {
  char filename[100];

  printf("== Simple C antivirus Scanner ==\n");
  printf("Enter a file name : to scan : ");
  scanf("%99s", filename);

  scan_file(filename);

  return 0;
}
