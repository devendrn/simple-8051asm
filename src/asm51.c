#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "literals.h"
#include "debug.h"
#include "parser.h"
#include "pack.h"

#define HEX_MIN_SIZE 128
#define HEX_MAX_SIZE 16384
#define SUBS_MIN_SIZE 128
#define SUBS_MAX_SIZE 16384
#define LABELS_MIN_SIZE 32
#define LABELS_MAX_SIZE 4096

char debug = 0; 

char mnemonic[16];
char operands[3][16];

struct asm_data {
  //int org[16];  // 16 org pages
  int org;        // org address
  int addr;       // current assembled byte
  
  // labels
  struct label *labels;      // change struct name to label_data
  int labels_filled;
  int labels_size;
  
  // assembled bytes
  unsigned char *hex;
  int hex_filled;
  int hex_size;

  // substitute data: 16-bit, 2 for mode, 14 for index
  // modes: 1=offset 2=addr16 3=addr11
  unsigned int *subs;
  int subs_filled;
  int subs_size;
} asmd;

// check hex array size and reallocate space if needed
void check_hex_bounds() {
  if(asmd.addr+4 < asmd.hex_size) {
    return;
  }

  asmd.hex_size *= 2;
  if (asmd.hex_size > HEX_MAX_SIZE) {
    printf("error: hex array too large");
    exit(0);
  }

  unsigned char *tmp_hex = realloc(asmd.hex, asmd.hex_size * sizeof(unsigned int));
  if (tmp_hex == NULL) {
    printf("error: memory reallocation failed for hex array");
    exit(0);
  } else {
    asmd.hex = tmp_hex;
  }
}

// check subs array size and reallocate space if needed
void check_subs_bounds() {
  if(asmd.subs_filled+4 < asmd.subs_size) {
    return;
  }

  asmd.subs_size *= 2;
  if (asmd.subs_size > SUBS_MAX_SIZE) {
    printf("error: substitute array too large");
    exit(0);
  }

  unsigned int *tmp_hex = realloc(asmd.subs, asmd.subs_size * sizeof(unsigned int));
  if (tmp_hex == NULL) {
    printf("error: memory reallocation failed for substitute array");
    exit(0);
  } else {
    asmd.subs = tmp_hex;
  }
}

// check labels array size and reallocate space if needed
void check_labels_bounds() {
  if(asmd.labels_filled+4 < asmd.labels_size) {
    return;
  }

  asmd.labels_size *= 2;
  if (asmd.labels_size > LABELS_MAX_SIZE) {
    printf("error: labels array too large");
    exit(0);
  }

  struct label *tmp_labels = realloc(asmd.labels, asmd.labels_size * sizeof(struct label));
  if (tmp_labels == NULL) {
    printf("error: memory reallocation failed for label array");
    exit(0);
  } else {
    asmd.labels = tmp_labels;
  }
}

// correct acall opcode and value
char set_addr11(unsigned char *opcode, unsigned char *addr11, int val, int current_addr) {
  // check if value in same page
  if (current_addr >> 11 != val >> 11) { 
    return 0;
  }
  // acall and ajmp takes 11 bits - 8 from operand, 3 from opcode
  // ajmp : opcode[a10 a9 a8 0 0001]
  // acall: opcode[a10 a9 a8 1 0001]
  // operand[a7 a6 a5 a4 a3 a2 a1]
  *opcode = ((val >> 8) << 5) + *opcode;
  *addr11 = val % 256;
  return 1;
}

// assemble into char array
void assemble(char *mnemonic, char operands[3][16]) {
  char er = 0;
  char j = 0;   // num of operands with data
  int k = -1;  // label pos if any
  int data[2] = {-1, -1};

  enum mnemonic_type mn = get_mnemonic_enum(mnemonic);
  if (mn == mn_invalid) {
    print_error(1, "Invalid mnemonic", mnemonic);
    er++;
  }

  struct operand op[4];
  for (char i = 0; i < 3; i++) {
    op[i] = get_operand_struct(operands[i]);
    if (op[i].type == op_invalid) {
      print_error(1, "Invalid operand", operands[i]);
      er++;
    }
    if (op[i].value > -1) {
      if (op[i].type == op_label) {
        k = i;
      }
      data[j++] = op[i].value;
    }
  }

  if (er > 0 || mn == mn_none) {
    return;
  }

  // set origin
  if (mn == mn_org) {
    if (asmd.org < 0) {
      if (op[0].type == op_direct && op[1].type == op_none &&
          op[2].type == op_none) {
        asmd.org = op[0].value;
      } else {
        print_error(1, "org requires a valid address", "");
      }
    } else {
      print_error(1, "org can only be set once", "");
    }
    return;
  }

  unsigned char opcode = get_opcode(mn, op);

  if (opcode == 0xa5) {
    print_error(1, "Invalid instruction", "");
    return;
  }
  
  // pack opcode
  asmd.hex[asmd.addr++] = opcode;
  
  check_hex_bounds();

  // **mov direct, direct stores data in reverse order
  if (opcode == 0x85) {
    int temp = data[1];
    data[1] = data[0];
    data[0] = temp;
  }

  // pack data
  if (data[0] > -1) {
    if (op[0].type == op_dptr) {
      // dptr takes 2 bytes
      asmd.hex[asmd.addr++] = data[0] >> 8;
      asmd.hex[asmd.addr++] = data[0] & 0x00ff;
      if (data[0] > 0xffff) {
        print_error(1, "Value cannot be larger than 65535", "");
      }
    } else {
      asmd.hex[asmd.addr++] = data[0];
      if ((mn == mn_acall || mn == mn_ajmp) && k < 0) {
        unsigned int current_addr = asmd.org >= 0 ? asmd.addr + asmd.org : asmd.addr;
        if (!set_addr11(&asmd.hex[asmd.addr - 2], &asmd.hex[asmd.addr], data[0], current_addr)) {
          print_error(1, "Address not in the same page", "");
        }
      } else if (data[0] > 0xff) {
        print_error(1, "Value cannot be larger than 255", "");
      }
    }
  }
  if (data[1] > -1) {
    asmd.hex[asmd.addr++] = data[1];
    if (data[1] > 0xff) {
      print_error(1, "Value cannot be larger than 255", "");
    }
  }

  // pack label details if any
  if (k > -1) {
    int pos = search_label(operands[k], asmd.labels);
    asmd.hex[asmd.addr - 1] = pos;
    switch (all_instructions[opcode].operands[k]) {
      case op_offset:
        asmd.subs[asmd.subs_filled++] = 0x4000 + asmd.addr - 1;
        break;
      case op_addr16:
        asmd.subs[asmd.subs_filled++] = 0x8000 + asmd.addr - 1;
        asmd.addr++;  // reserve next space for a byte
        break;
      case op_addr11:
        asmd.subs[asmd.subs_filled++] = 0xc000 + asmd.addr - 1;
        break;
      default:
        break;
    }
    check_subs_bounds();
  }

  if (debug) {
    print_instruction(debug_line, mnemonic, operands, mn, op, opcode, data);
  }
}

// substitute undefined data(labels) with offset or address
void substitute_labels() {
  for (int i = 0; i < asmd.subs_filled; i++) {
    char type = asmd.subs[i] >> 14;
    int index = asmd.subs[i] & 0x3fff;
    char label_index = asmd.hex[index];
    int loc = asmd.labels[label_index].addr;
    char *name = asmd.labels[label_index].name;

    if (loc < 0) {
      print_error(0, "Label not defined", name);
    }

    switch (type) {
      case 1:; // offset
        int offset = loc - index;
        if (offset > 127 || offset < -128) {
          print_error(0, "Label offset out of range", name);
        }
        asmd.hex[index] = offset < 0 ? 255 + offset : offset - 1;
        break;
      case 2: // addr16
        asmd.hex[index] = loc >> 8;
        asmd.hex[index + 1] = loc & 0x00ff;
        break;
      case 3: // addr11
        if (!set_addr11(&asmd.hex[index - 1], &asmd.hex[index], loc + asmd.org + 1, index + asmd.org)) {
          print_error(0, "Label is not in the same page for ACALL", name);
        }
      default:
        break;
    }
  }
}

int main(int argc, char **argv) {
  char *file_in;
  char *file_out = "a.hex";

  char arg_mode = 'i';
  char got_fi = 0;
  char got_fo = 0;
  for (int i = 1; i<argc; i++) {
    if (argv[i][0] == '-') {
      switch(argv[i][1]){
        case 'h':
          printf("usage: asm51 [input file] -o [output file]\n -h  help\n -v  version\n -o  output file\n -d  debug\n");
          return 1;
        case 'o':
          arg_mode = 'o';
          break;
        case 'v':
          #ifdef VERSION
          printf("simple-asm51: version %s\n", VERSION);
          #endif
          return 1;
        case 'd':
          debug = 1;
          break;
        default:
          printf("error: invalid option -%c\nuse -h for help\n",argv[i][1]);
          return 0;
      }
    } else {
      if (arg_mode == 'i') {
        if(got_fi){
          printf("error: multiple input files\n");
          return 0;
        }
        file_in = argv[i];
        got_fi = 1;
      } else if (arg_mode == 'o') {
        if(got_fo){
          printf("error: multiple output files\n");
          return 0;
        }
        file_out = argv[i];
        got_fo = 1;
        arg_mode = 'i';
      }
    }
  }

  if (!got_fi) {
    printf("error: no input file\n");
    return 0;
  }

  FILE *asm_file = fopen(file_in, "r");
  if (asm_file == NULL) {
    printf("error: file '%s' not found\n", file_in);
    return 0;
  }

  asmd.org = -1;
  asmd.addr = 0;
  asmd.hex_filled = 0;
  asmd.subs_filled = 0;
  asmd.labels_filled = 0;
  asmd.hex_size = HEX_MIN_SIZE;
  asmd.subs_size = SUBS_MIN_SIZE;
  asmd.labels_size = LABELS_MIN_SIZE;

  asmd.hex = calloc(HEX_MIN_SIZE, sizeof(char));
  asmd.subs = calloc(SUBS_MIN_SIZE, sizeof(int));
  asmd.labels = calloc(LABELS_MIN_SIZE, sizeof(struct label));

  if (asmd.labels == NULL || asmd.hex == NULL) {
    printf("error: memory allocation failed");
    fclose(asm_file);
    return 0;
  }

  if (debug) {
    printf(" - parsed instructions ---------------");
  }

  while (!feof(asm_file)) {
    debug_line++;
    if (parse_line(asm_file, mnemonic, operands, asmd.addr, asmd.labels)) {
      assemble(mnemonic, operands);
    }
    check_labels_bounds(); // to-do - move this
  }
  asmd.hex_filled = asmd.addr;


  if (debug) {
    print_labels(asmd.labels);
    print_subs_array(asmd.subs, asmd.subs_filled);
    printf(" before substitute:");
    print_hex_array(asmd.hex, asmd.hex_filled);
  }

  substitute_labels();

  if (debug) {
    printf("\n after substitute:");
    print_hex_array(asmd.hex, asmd.hex_filled);
  }

  fclose(asm_file);

  if (debug_errors == 0) {
    pack_ihex(file_out, asmd.hex, asmd.hex_filled, asmd.org);
  }

  free(asmd.labels);
  free(asmd.hex);
  free(asmd.subs);

  return 0;
}
