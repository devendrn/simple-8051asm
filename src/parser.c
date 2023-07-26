#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "literals.h"
#include "debug.h"

// check if string is a valid label
int check_label(char *label) {
  // first char of label must be an alphabet or _
  // other chars must be alphanumeric or _
  char c = label[0];
  if (isalpha(c) || c == '_') {
    for (int i = 1; label[i] != '\0'; i++) {
      c = label[i];
      if (!isalnum(c) && c != '_') {
        return 0;
      }
    }
    return 1;
  }
  return 0;
}

// push defined labels with their address
void push_label_src(char *label, int addr, struct labels *all_labels) {
  if (!check_label(label)) {
    print_error(1, "Invalid label name", label);
    return;
  }
  int i;
  for (i = 0; all_labels[i].name[0] != '\0'; i++) {  // check if label already exists
    if (!strcmp(all_labels[i].name, label)) {
      if (all_labels[i].addr >= 0) {
        print_error(1, "Label already defined previously", label);
      }
      all_labels[i].addr = addr;
      return;
    }
  }
  memcpy(all_labels[i].name, label, strlen(label) + 1);
  all_labels[i].addr = addr;
}

// find label and return its index 
// insert label and return new index if not found
// address is -1 when pushing (replaced later when src is found)
int search_label(char *label, struct labels *all_labels) {
  int i;
  for (i = 0; i < all_labels[i].name[0] != '\0'; i++) {
    if (!strcmp(label, all_labels[i].name)) {
      return i;
    }
  }
  memcpy(all_labels[i].name, label, strlen(label));
  all_labels[i].addr = -1;
  return i;
}

// split lines into sub strings 
char parse_line(FILE *file, char *mnemonic, char operands[3][16], int addr, struct labels *label_array) {
  mnemonic[0] = '\0';
  operands[0][0] = operands[1][0] = operands[2][0] = '\0';
  char operand_count = 0;
  char part = 0;  // 0-label 1-mnemonic 2-operands
  char after_space = 0, use_case = 0;
  unsigned char mi = 0, oi = 0;
  char c;
  while (1) {
    c = fgetc(file);
    
    // skip to eol if comment
    // close strings before breaking out 
    if (c == ';') {
      while (c != '\n') {
        c = fgetc(file);
      }
    }
    if (c == '\n' || feof(file)) {
      mnemonic[mi] = '\0';
      operands[operand_count][oi] = '\0';
      break;
    }

    // skip spaces and tabs
    if (c == ' ' || c == '\t') {
      after_space = 1;
      continue;
    }
    
    // check for label/mnemonic
    if (part < 2){
      if (c == ':' && part == 0) {
        mnemonic[mi] = '\0';
        push_label_src(mnemonic, addr, label_array);
        mnemonic[0] = '\0';
        mi = 0;
        part = 1;
        continue;
      }
      if (after_space && mi > 0) {
        mnemonic[mi] = '\0';
        part = 2;
      } else {
        mnemonic[mi++] = tolower(c);
      }
    }

    // check for operands
    if (part == 2) {
      if (c == ',') {
        operands[operand_count][oi] = '\0';
        oi = 0;
        if (operand_count > 1) {
          print_error(1, "Too many operands", "");
          while (c != '\n') {
            c = fgetc(file);
          }
          return 0;
        }
        operand_count++;
        continue;
      }
      if (c == '\'') {
        use_case = 1 - use_case;  // preserve case for chars inside ' '
      }
      operands[operand_count][oi++] = use_case ? c : tolower(c);
    }
    after_space = 0;
  }
  return 1;
}

// compare mnemonic and operand enum with table
// return opcode of matching instruction
// return 0xa5 if not found
unsigned char get_opcode(enum mnemonic_type mn, struct operand op[]) {
  for (int i = 0; i < 256; i++) {
    if (mn != all_instructions[i].mnemonic) {
      continue;
    }

    char j;
    for (j = 0; j < 3; j++) {
      enum operand_type search = all_instructions[i].operands[j];

      // labels can replace offset,addr16,addr11
      if (op[j].type == op_label && search <= op_offset && search >= op_addr11) {
        continue;
      }

      // offset, addr11, addr16 can replace direct values
      // addr11 opcode is corrected later, for now it returns first match
      // ajmp:0x01   acall:0x11
      if (op[j].type == op_direct && (search == op_addr11 || search == op_addr16 || search == op_offset)) {
        continue;
      }

      if (op[j].type != search) {
        break;
      }
    }
    if (j > 2) {
      return i;
    }
  }
  return 0xa5;
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
// return address value if sfr
int check_sfr(char *word, struct operand *out) {
  // check for sfr bit - bit.index
  int l = strlen(word);
  char last = l > 3 ? word[l - 1] : 0;
  if (word[l - 2] == '.' && last <= '7' && last >= '0') {
    for (char i = 0; i <= sfr_bitaddr_end; i++) {  // 0-10 all_sfrs are bit addressable
      // labels can replace offset,addr16,addr11
      if (str_cmp(all_sfrs[i].name, word, '.')) {
        out->type = op_bit;
        out->value = all_sfrs[i].addr + last - '0';
        return 1;
      }
    }
    out->type = op_invalid;
    return 1;
  }
  
  // check for sfr bit - named
  for (char i = 0; i <= sfr_bit_end; i++) {
    if (!strcmp(all_sfr_bits[i].name, word)) {
      out->type = op_bit;
      out->value = all_sfr_bits[i].addr;
      return 1;
    }
  }

  // check for sfr
  for (char i = 0; i <= sfr_end; i++) {
    if (!strcmp(all_sfrs[i].name, word)) {
      out->type = op_direct;
      out->value = all_sfrs[i].addr;
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

// string to mnemonic enum
enum mnemonic_type get_mnemonic_enum(char *word) {
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
