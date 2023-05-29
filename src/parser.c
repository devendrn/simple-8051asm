#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "literals.h"

// return opcode of matching instruction
unsigned char get_opcode(enum mnemonic_type mn, struct operand op[]) {
  for (int i = 0; i < 256; i++) {
    if (mn != all_instructions[i].mnemonic) {
      continue;
    }
    int j;
    for (j = 0; j < 3; j++) {
      enum operand_type search = all_instructions[i].operands[j];
      if (op[j].type == op_label && search <= op_offset &&
          search >= op_addr11) {  // labels can replace offset,addr16,addr11
        continue;
      }
      if (op[j].type == op_direct &&
          search == op_addr11) {  // adddr11 takes direct values as well
        // this will give out #11, the correct opcode will be calculated from
        // the value later
        continue;
      }
      if (op[j].type != search) {
        break;
      }
    }
    if (j > 2) {
      return i;  // return instruction opcode
    }
  }
  return 0xa5;  // no matching instruction found
}

// check if string is a valid label
int check_label(char *label) {
  if (isalpha(label[0])) {  // first char of label must be an alphabet
    for (int i = 1; label[i] != '\0';
         i++) {  // other chars must be alphanumeric
      if (!isalnum(label[i])) {
        return 0;
      }
    }
    return 1;
  }
  return 0;
}

// compare string up to specified char
int str_cmp(const char *word_l, const char *word_s, char end) {
  for (int i = 0; word_l[i] != end && word_l[i] != '\0'; i++) {
    if (word_l[i] != word_s[i]) {
      return 0;
    }
  }
  return 1;
}

// check if operand is a sfr
int check_sfr(char *word, struct operand *out) {
  // check for sfr bit - bit.index
  int l = strlen(word);
  char last = l > 3 ? word[l - 1] : 0;
  if (word[l - 2] == '.' && last <= '7' && last >= '0') {
    for (int i = 0; i < 11; i++) {  // 0-10 defined_vars are bit addressable
      // labels can replace offset,addr16,addr11
      if (str_cmp(defined_vars[i].name, word, '.')) {
        out->type = op_bit;
        out->value = defined_vars[i].addr + last - '0';
        return 1;
      }
    }
    out->type = op_invalid;
    return 1;
  }

  // check for sfr
  for (int i = 0; i < 21; i++) {
    if (!strcmp(defined_vars[i].name, word)) {
      out->type = op_direct;
      out->value = defined_vars[i].addr;
      return 1;
    }
  }
  return 0;
}

// converts the following formats of string to int
// 12 12h 0x12 -12 +12 'a' 1110b
int str_to_int(int *out_val, char *str) {
  unsigned char base = 10;
  unsigned char start = 0;
  unsigned char end = strlen(str) - 1;

  // ascii - 'a'
  if (end == 2 && str[start] == str[end] && str[end] == '\'') {
    *out_val = (int)str[start + 1];
    return 1;
  }

  // hex - 0xab, 12h
  if (str[end] == 'h') {
    base = 16;
    end--;
  } else if (str[start] == '0' && str[start + 1] == 'x') {
    base = 16;
    start += 2;
  }

  // binary - 10110b
  else if (str[end] == 'b') {  // binary
    base = 2;
    end--;
  }

  for (int i = end, j = 1; i >= start; i--, j *= base) {
    char w = str[i];

    if (w == '-') {  // negative index from 0xff
      if (i == start && *out_val < 256) {
        *out_val = 256 - *out_val;
        return 1;
      }
      return 0;  // sign should always be at start
    }

    if (w == '+') {  // plus will be ignored
      if (i == start) {
        return 1;
      }
      return 0;
    }

    int iw = w - '0';  // char to int (0-9)
    if (base == 16 && w <= 'f' && w >= 'a') {
      iw = w - 87;  // hex char to int (a-f)
    } else if (iw < 0 || iw > 9 || (base == 2 && iw > 1)) {  // invalid char
      return 0;
    }
    *out_val += iw * j;
  }
  return 1;
}

// get a word from line
char get_token(FILE *file, char *out_str, char end_char) {
  char ch;
  int i = 0;
  char case_sensitive = 0;
  while (1) {
    ch = fgetc(file);
    if (ch == ';') {
      // if comment skip to eol
      while (ch != '\n') {
        ch = fgetc(file);
      }
      break;
    } else if ((ch == ' ' || ch == '\t') && (end_char == ',' || i == 0)) {
      // skip spaces and tabs if scanning for operands or if mnemonic not found
      continue;
    } else if (ch == end_char || ch == ':' || ch == '\n' || feof(file)) {
      // break (check last char outside the func to detect label)
      // break if eol/eof
      break;
    } else if (ch == '\'') {
      // toggle case sensitive for '' operands
      case_sensitive = 1 - case_sensitive;
    }
    out_str[i++] = case_sensitive ? ch : tolower(ch);
  }
  out_str[i] = '\0';  // end string
  return ch;          // return last char
}

// get mnemonic enum from string
enum mnemonic_type get_mnemonic_enum(char *word) {
  // check through all possible mnemonics
  for (int i = 0; i < mn_invalid; i++) {
    if (!strcmp(all_mnemonics[i], word)) {
      return i;
    }
  }
  return mn_invalid;  // return invalid if no matching mnemonic
}

// get operand
struct operand get_operand_struct(char *word) {
  struct operand out = {op_invalid, 0x00};

  // check through all possible operands
  for (int i = 0; i <= op_none; i++) {
    if (!strcmp(all_operands[i], word)) {
      out.type = i;
      out.value = -1;
      return out;
    }
  }

  // check for special function registers like acc,b,psw etc
  if (check_sfr(word, &out)) {
    return out;
  }

  // check for label
  if (check_label(word)) {
    out.type = op_label;
    return out;
  }

  // check for value based operands
  int start = 0;
  if (word[0] == '#') {  // immed operand
    out.type = op_immed;
    start = 1;
  }
  if (str_to_int(&out.value, &word[start])) {
    if (out.type != op_immed) {
      out.type = op_direct;
    }
    return out;
  }

  out.type = op_invalid;
  return out;
}
