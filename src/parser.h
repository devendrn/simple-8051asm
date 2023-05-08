#ifndef PARSER_H
#define PARSER_H

#include "literals.h"
#include <stdio.h>

unsigned char get_opcode(enum mnemonic_type mn,struct operand op[]);

int check_label(char *label);

int str_cmp(const char *word_l,const char *word_s,char end);

int check_sfr(char *word,struct operand *out);

int str_to_int(int *out_val,char *str);

char get_token(FILE *file,char *out_str,char end_char);

enum mnemonic_type get_mnemonic_enum(char *word);

struct operand get_operand_struct(char *word);


#endif
