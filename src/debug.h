#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

extern unsigned int debug_line;
extern unsigned int debug_errors;

void print_error(int line_true, char *error, char *element);

#endif
