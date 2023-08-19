#ifndef ASSEMBLER_H 
#define ASSEMBLER_H

#include "literals.h"

// max size can be more, but assuming general use

#define HEX_MIN_SIZE 128
#define HEX_MAX_SIZE 32768 // abs max = 65536

#define SUBS_MIN_SIZE 128
#define SUBS_MAX_SIZE 16384 // abs max = 65536

#define LABELS_MIN_SIZE 32
#define LABELS_MAX_SIZE 2048 // abs max = 16384

#define ORGS_MIN_SIZE 16
#define ORGS_MAX_SIZE 1024 // abs max = 65536
#define ORG_DEFAULT_ADDR 0x0000

struct subs_data {
  // 16-bit, 2 for mode, 14 for label index 
  // modes: 1=offset 2=addr16 3=addr11
  unsigned int mode_and_id;
  unsigned int hex_addr;
  unsigned int addr;
};

struct orgs_data {
  unsigned int addr;
  unsigned int hex_start;
  unsigned int hex_end;
};

extern struct asm_data {
  // current assembled byte
  unsigned int addr;
  
  // [0] = org addr, [1] - hex start, [2] - hex end
  unsigned int (*orgs)[3]; 
  int orgs_filled;
  int orgs_size;
  
  // labels
  struct label *labels;
  int labels_filled;
  int labels_size;
  
  // assembled bytes
  unsigned char *hex;
  int hex_filled;
  int hex_size;

  // substitute data
  struct subs_data *subs;
  int subs_filled;
  int subs_size;
} asmd;


void initialize_asmd();

void check_hex_bounds();
void check_subs_bounds();
void check_labels_bounds();
void check_orgs_bounds();

int get_current_addr(int current_org_page);

char set_addr11(unsigned char *opcode, unsigned char *addr11, int val, int current_addr);

void assemble(char *mnemonic, char operands[3][16]);

void substitute_labels();

#endif
