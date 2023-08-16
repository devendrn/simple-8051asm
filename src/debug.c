#include <stdio.h>

#include "debug.h"
#include "literals.h"

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

void print_instruction(unsigned int debug_line, char *mnemonic, char operands[][16], char mn, struct operand *op, unsigned char opcode, int data[2]){
    printf("\n%3d| %02x", debug_line, opcode);
    data[0] > -1 ? printf(" %02x", data[0]) : printf("   ");
    data[1] > -1 ? printf(" %02x", data[1]) : printf("   ");
    if (mn != mn_none) {
      //printf(" |%6s >%6s |", mnemonic, all_mnemonics[mn]);
      //printf(" %8s > %8s:%4d |", operands[0], all_operands[op[0].type], op[0].value);
      //printf(" %8s > %8s:%4d |", operands[1], all_operands[op[1].type], op[1].value);
      //printf(" %8s > %8s:%4d |", operands[2], all_operands[op[2].type], op[2].value);
      printf(" | %5s", all_mnemonics[mn]);
      for (int i = 0; i < 3; i++) {
        if (op[i].type != op_none) {
          //printf(" %8s:%4d ", all_operands[op[i].type], op[i].value);
          if (i>0) {
            printf(",");
          }
          if (op[i].value < 0) {
            printf(" %s", all_operands[op[i].type]);
          } else {
            // op enum are in order addr11, addr16, offset, bit, direct, immed, label
            char *names[] = {"addr11", "addr16", "offset", "bit", "direct", "immed", "label"};
            char j = op[i].type - op_addr11;
            printf(" %s(%d)", names[j], op[i].value);
          }
        }
      }
    }
}

void print_hex_array(unsigned char *hex, int hex_size) {
  printf("\n - hex array -----------\n");
  for (int i = 0; i < hex_size; i++) {
    printf(" %02x", hex[i]);
    if ((i + 1) % 8 == 0) {
      printf("\n");
    }
  }
  printf("\n");
}

void print_subs_array(unsigned int *subs, int subs_size) {
  printf("\n - substitutes --\n");
  char *modes[] = {"offset", "addr16", "addr11"};
  for (int i = 0; i < subs_size; i++) {
    char mode = subs[i] >> 14;
    printf(" 0x%04x  %01x=%s\n", subs[i] & 0x3fff, mode, modes[mode - 1]);
  }
  printf("\n");
}

void print_labels(struct label all_labels[]) {
  printf("\n\n - labels --------");
  for (int i = 0; all_labels[i].name[0] != '\0'; i++) {
    printf("\n%2d:%8s 0x%04x", i, all_labels[i].name, all_labels[i].addr);
  }
  printf("\n");
}
