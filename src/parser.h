#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

#include "literals.h"
#include "debug.h"


int str_cmp(const char *word_l, const char *word_s, char end);

int str_to_int(int *out_val, char *str);

int check_label(char *label);

int check_sfr(char *word, struct operand *out);

void push_label_src(char *label, int addr, struct label *all_labels);

int search_label(char *label, struct label *all_labels);

char parse_line(FILE *file, char *mnemonic, char operands[3][16], int addr, struct label *label_array);


unsigned char get_opcode(enum mnemonic_type mn, struct operand op[]);

enum mnemonic_type get_mnemonic_enum(char *word);

struct operand get_operand_struct(char *word);


#endif
