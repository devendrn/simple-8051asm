#include <stdio.h>

#include "debug.h"
#include "assembler.h"
#include "literals.h"

char debug = 0; 
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

void print_instruction_start() {
  printf(" ┌ parsed instruction ────────────────\n");
  printf(" │ line  assembled  parsed");
}

void print_instruction(unsigned int debug_line, char *mnemonic, char operands[][16], char mn, struct operand *op, unsigned char opcode, int data[2]){
    printf("\n │%5d  %02x", debug_line, opcode);
    data[0] > -1 ? printf(" %02x", data[0]) : printf("   ");
    data[1] > -1 ? printf(" %02x", data[1]) : printf("   ");
    if (mn != mn_none) {
      printf("   %-5s", all_mnemonics[mn]);
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

void print_instruction_end() {
  printf("\n └────────────────────────────────────\n");
}

void print_hex_array(unsigned char *hex, int hex_size) {
  printf("\n ┌ hex array ──────────────┐");
  for (int i = 0; i < hex_size; i++) {
    if (i % 8 == 0) {
      printf("\n │");
    }
    printf(" %02x", hex[i]);
  }
  printf("\n └─────────────────────────┘\n");
}

void print_subs_table(struct subs_data *subs, int subs_size) {
  printf(" ┌ substitutes ──────────┐\n");
  printf(" │ addr    type    label │\n");
  char *modes[] = {"offset", "addr16", "addr11"};
  for (int i = 0; i < subs_size; i++) {
    char mode = subs[i].mode_and_id >> 14;
    int label_id= subs[i].mode_and_id & 0x3fff;
    int addr = subs[i].addr;
    printf(" │ 0x%04x  %6s  %-5d │\n", addr, modes[mode-1], label_id);
  }
  printf(" └───────────────────────┘\n");
}

void print_labels_table(struct label all_labels[]) {
  printf(" ┌ labels ──────────────────┐\n");
  printf(" │ id   name         addr   │\n");
  for (int i = 0; all_labels[i].name[0] != '\0'; i++) {
    printf(" │ %-4d %-12s 0x%04x │\n", i, all_labels[i].name, all_labels[i].addr);
  }
  printf(" └──────────────────────────┘\n");
}

void print_orgs_table(unsigned int (*orgs)[3], unsigned int orgs_filled) {
  printf(" ┌ orgs ─────────────────┐\n");
  printf(" │ addr    start  end    │\n");
  for (int i = 0; i < orgs_filled; i++) {
    printf(" │ 0x%04x  0x%04x 0x%04x │\n", orgs[i][0], orgs[i][1], orgs[i][2]);
  }
  printf(" └───────────────────────┘");
}

void print_packed_table(unsigned char *hex, unsigned int (*orgs)[3], unsigned int orgs_filled) {
  printf("\n ┌ packed ─────────────────────────┐");
  printf("\n │ addr    data                    │");
  for (int i = 0; i < orgs_filled; i++) {
    int addr = orgs[i][0];
    int start = orgs[i][1];
    int end = orgs[i][2];
    for (int j = start; j<=end; j++) {
      if ((j-start)%8 == 0) {
        printf("\n │ 0x%04x ", addr);
        addr += 0x0008;
      }
      printf(" %02x", hex[j]);
    }
  }
  printf("\n └─────────────────────────────────┘\n");
}
