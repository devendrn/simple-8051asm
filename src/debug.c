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
            printf(" %s(%d)", all_operands[op[i].type], op[i].value);
          }
        }
      }
    }
}

void print_hexarray(unsigned char out_hex[][2]) {
  printf("\n - unsubstitued hex --------------------\n");
  for (int i = 0; out_hex[i][1] != 255; i++) {
    printf(" %02x:%01x", out_hex[i][0], out_hex[i][1]);
    if ((i + 1) % 8 == 0) {
      printf("\n");
    }
  }
  printf("\n");
}

void print_hexarray_values(unsigned char out_hex[][2]) {
  printf("\n - final hex -----------\n");
  for (int i = 0; out_hex[i][1] != 255; i++) {
    printf(" %02x", out_hex[i][0]);
    if ((i + 1) % 8 == 0) {
      printf("\n");
    }
  }
  printf("\n");
}

void print_labels(struct labels all_labels[]) {
  printf("\n\n - labels --------");
  for (int i = 0; all_labels[i].name[0] != '\0'; i++) {
    printf("\n%2d:%8s 0x%04x", i, all_labels[i].name, all_labels[i].addr);
  }
  printf("\n");
}
