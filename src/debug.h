#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

#include "literals.h"

extern unsigned int debug_line;
extern unsigned int debug_errors;

void print_error(int line_true, char *error, char *element);

void print_instruction(unsigned int debug_line, char *mnemonic, char operands[][16], char mn, struct operand *op, unsigned char opcode, int data[2]);
void print_hexarray(unsigned char out_hex[][2]);
void print_hexarray_values(unsigned char out_hex[][2]);
void print_labels(struct labels all_labels[]);

#endif
