#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "literals.h"
#include "debug.h"
#include "parser.h"
#include "pack.h"

// #define DEBUG

char mnemonic[16];
char operands[3][16];
struct labels all_labels[256];

int org_addr = -1;   // origin address
int addr = 0;        // byte address

// [[data,substitute]...]
// substitute = 0:skip  1:offset  2:addr16  255:end
unsigned char out_hex[512][2];


// correct acall opcode and value
unsigned char set_acall_addr(unsigned char *opcode, unsigned char *addr11, int val, int current_addr) {
  // check if value in same page
  if (current_addr >> 11 == val >> 11) { 
    // acall takes 11 bits - 8 from operand, 3 from opcode
    // opcode[a10 a9 a8 1 0001]  operand[a7 a6 a5 a4 a3 a2 a1]
    *opcode = ((val >> 8) << 5) + 0x11;
    *addr11 = val % 256;
    return 1;
  }
  return 0;
}

// assemble into char array
void assemble(unsigned char out[][2], char *mnemonic, char operands[3][16]) {
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
    if (org_addr < 0) {
      if (op[0].type == op_direct && op[1].type == op_none &&
          op[2].type == op_none) {
        org_addr = op[0].value;
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
  out[addr++][0] = opcode;

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
      int msb = data[0] >> 8;
      out[addr++][0] = msb;
      out[addr++][0] = data[0] - (msb << 8);
      if (data[0] > 0xffff) {
        print_error(1, "Value cannot be larger than 65535", "");
      }
    } else {
      out[addr++][0] = data[0];
      if (mn == mn_acall && k < 0) {  // acall label shall be processed later
        unsigned int current_addr = org_addr >= 0 ? addr + org_addr : addr;
        if (!set_acall_addr(&out[addr - 2][0], &out[addr][0], data[0],
                            current_addr)) {
          print_error(1, "Address not in the same page", "");
        }
      } else if (data[0] > 0xff) {
        print_error(1, "Value cannot be larger than 255", "");
      }
    }
  }
  if (data[1] > -1) {
    out[addr++][0] = data[1];
    if (data[1] > 0xff) {
      print_error(1, "Value cannot be larger than 255", "");
    }
  }

  // pack label details if any
  if (k > -1) {
    int pos = search_label(operands[k], all_labels);
    out[addr - 1][0] = pos;
    enum operand_type parse_type = all_instructions[opcode].operands[k];
    if (parse_type == op_offset) {
        out[addr - 1][1] = 1;
    } else if (parse_type == op_addr16) {
        out[addr - 1][1] = 2;
        addr++;  // reserve next space for a byte
    } else if (parse_type == op_addr11) {
        out[addr - 1][1] = 3;
    }
  }

#ifdef DEBUG
  printf("\n%2d| %02x", debug_line, opcode);
  data[0] > -1 ? printf(" %02x", data[0]) : printf("   ");
  data[1] > -1 ? printf(" %02x", data[1]) : printf("   ");
  if (mn != mn_none) {
    printf(" |%6s >%6s |", mnemonic, all_mnemonics[mn]);
    printf(" %8s > %8s:%4d |", operands[0], all_operands[op[0].type], op[0].value);
    printf(" %8s > %8s:%4d |", operands[1], all_operands[op[1].type], op[1].value);
    printf(" %8s > %8s:%4d |", operands[2], all_operands[op[2].type], op[2].value);
  }
#endif
}

// substitute undefined data(labels) with offset or address
void substitute_labels(unsigned char hex_array[][2]) {
  int i = -1;
  while (1) {
    unsigned char type = hex_array[++i][1];

    if (type == 255) {  // end of hex array
      break;
    }

    if (type == 0) {  // defined data - skip
      continue;
    }

    int loc = all_labels[hex_array[i][0]].addr;
    if (loc < 0) {
      print_error(0, "Label not defined", all_labels[hex_array[i][0]].name);
    }

    if (type == 1) {  // offset
      int offset = loc - i;
      if (offset > 127 || offset < -128) {
        print_error(0, "Label offset out of range",
                    all_labels[hex_array[i][0]].name);
      }
      hex_array[i][0] = offset < 0 ? 255 + offset : offset - 1;
    } else if (type == 2) { // addr16
        hex_array[i][0] = loc >> 8;
        hex_array[i + 1][0] = loc - (hex_array[i][0] << 8);
        i++;
    } else if (type == 3) {
      if (!set_acall_addr(&hex_array[i - 1][0], &hex_array[i][0],
                          loc + org_addr, i + org_addr)) {
        print_error(0, "Label is not in the same page for ACALL",
                    all_labels[hex_array[i][0]].name);
      }
    }

  }
}

int main(int argc, char **argv) {
  char *file_in;
  char *file_out = "a.hex";

  if (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'h') {
    printf("Usage:\nasm51 [input_file] -o [output_file]\n");
    return 1;
  } else if (argc == 2 && argv[1][0] != '-') {
    printf("%s", argv[1]);
    file_in = argv[1];
  } else if (argc == 4 && !strcmp("-o", argv[2])) {
    file_in = argv[1];
    file_out = argv[3];
  } else {
    printf("Invalid arguments: Use -h for help\n");
    return 0;
  }

  FILE *asm_file = fopen(file_in, "r");
  if (asm_file == NULL) {
    printf("File '%s' not found\n", file_in);
    return 0;
  }

  // process instructions line by line
  while (!feof(asm_file)) {
    debug_line++;
    if (parse_line(asm_file, mnemonic, operands, addr, all_labels)) {
      assemble(out_hex, mnemonic, operands);
    }
  }
  out_hex[addr][1] = 255;
  fclose(asm_file);

#ifdef DEBUG
  printf("\n\nLabels:");
  for (int i = 0; all_labels[i].name[0] != '\0'; i++) {
    printf("\n%2d:%7s | %04x", i, all_labels[i].name, all_labels[i].addr);
  }
  printf("\n\nHex array:\n");
  for (int i = 0; out_hex[i][1] != 255; i++) {
    printf(" %02x %01x |", out_hex[i][0], out_hex[i][1]);
    if ((i + 1) % 8 == 0) {
      printf("\n");
    }
  }
#endif

  substitute_labels(out_hex);

#ifdef DEBUG
  printf("\n\nFinal hex:\n");
  for (int i = 0; out_hex[i][1] != 255; i++) {
    printf(" %02x", out_hex[i][0]);
    if ((i + 1) % 8 == 0) {
      printf("\n");
    }
  }
  printf("\n");
#endif

  if (debug_errors == 0) {
    pack_ihex(file_out, out_hex, org_addr);
  }

  return 0;
}
