#include <stdio.h>

#include "debug.h"

unsigned int debug_line = 0;
unsigned int debug_errors = 0;

void print_error(int line_true, char *error, char *element) {
  printf("\n");
  if (line_true) {
    printf("Line %-3d | ", debug_line);
  }
  printf("error: %s", error);
  if (element[0] != '\0') {
    printf(" '%s'", element);
  }
  debug_errors++;
}
