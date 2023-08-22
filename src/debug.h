#ifndef DEBUG_H
#define DEBUG_H

#include "literals.h"
#include "assembler.h"

extern char debug; 
extern unsigned int debug_line;
extern unsigned int debug_errors;

void print_error(int line_true, char *error, char *element);

void print_instruction_start();
void print_instruction(unsigned int debug_line, char *mnemonic, char operands[][16], char mn, struct operand *op, unsigned char opcode, int data[2]);
void print_instruction_end();

void print_hex_array(unsigned char *hex, int hex_size);

void print_subs_table(struct subs_data *subs, int subs_size);
void print_labels_table(struct label all_labels[]);
void print_orgs_table(unsigned int (*orgs)[3], unsigned int orgs_filled);
void print_packed_table(unsigned char *hex, unsigned int (*orgs)[3], unsigned int orgs_filled);

#endif
