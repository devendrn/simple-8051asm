#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "assembler.h"
#include "literals.h"
#include "debug.h"
#include "parser.h"

struct asm_data asmd = {
  .addr = 0, .orgs_filled = 0, .hex_filled = 0, .subs_filled = 0, .labels_filled = 0,
  .hex_size = HEX_MIN_SIZE,
  .subs_size = SUBS_MIN_SIZE,
  .labels_size = LABELS_MIN_SIZE,
  .orgs_size = ORGS_MIN_SIZE,
};

// get current instruction addr using hex addr and org addr
int get_current_addr(int current_org_page) {
  return asmd.orgs[current_org_page][0] + (asmd.addr - asmd.orgs[current_org_page][1]);
}

// initialize asmd data
void initialize_asmd() {
  asmd.hex = calloc(HEX_MIN_SIZE, sizeof(char));
  asmd.subs = calloc(SUBS_MIN_SIZE, sizeof(int));
  asmd.labels = calloc(LABELS_MIN_SIZE, sizeof(struct label));
  asmd.orgs = calloc(ORGS_MIN_SIZE, 3 * sizeof(unsigned int));
  if (asmd.labels == NULL || asmd.hex == NULL || asmd.orgs == NULL || asmd.subs == NULL) {
    printf("error: memory allocation failed");
    exit(0);
  }

  asmd.orgs[0][0] = ORG_DEFAULT_ADDR;
  asmd.orgs[0][1] = 0;
  asmd.orgs_filled++;

}

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

  unsigned char *tmp_hex = realloc(asmd.hex, asmd.hex_size * sizeof(unsigned char));
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

  struct subs_data *tmp_subs = realloc(asmd.subs, asmd.subs_size * sizeof(struct subs_data));
  if (tmp_subs== NULL) {
    printf("error: memory reallocation failed for substitute array");
    exit(0);
  } else {
    asmd.subs = tmp_subs;
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

// check orgs array size and reallocate space if needed
void check_orgs_bounds() {
  if(asmd.orgs_filled+4 < asmd.orgs_size) {
    return;
  }

  asmd.orgs_size *= 2;
  if (asmd.orgs_size > ORGS_MAX_SIZE) {
    printf("error: org array too large");
    exit(0);
  }

  unsigned int (*tmp_orgs)[3] = realloc(asmd.orgs, asmd.orgs_size * 3 * sizeof(unsigned int));
  if (tmp_orgs == NULL) {
    printf("error: memory reallocation failed for label array");
    exit(0);
  } else {
    asmd.orgs = tmp_orgs;
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
  switch(mn) {
    case mn_org:
      if (op[0].type == op_direct && op[1].type == op_none && op[2].type == op_none) {
        asmd.orgs[asmd.orgs_filled-1][2] = asmd.addr > 0 ? asmd.addr - 1 : 0;  // end prev org page
        asmd.orgs[asmd.orgs_filled][0] = op[0].value;
        asmd.orgs[asmd.orgs_filled][1] = asmd.addr;
        asmd.orgs_filled++;
      } else {
        print_error(1, "org requires a valid address", "");
      }
      check_orgs_bounds();
      return;
    case mn_db:
      if (op[0].type == op_direct && op[1].type == op_none && op[2].type == op_none) {
        asmd.hex[asmd.addr++] = op[0].value;
      } else {
        print_error(1, "db requires valid data", "");
      }
      return;
    default:
      break;
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
        // todo - verify this
        unsigned int current_addr = get_current_addr(asmd.orgs_filled - 1);
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
    int label_id = search_label(operands[k], asmd.labels);
    asmd.subs[asmd.subs_filled].hex_addr = asmd.addr - 1;
    asmd.subs[asmd.subs_filled].addr = get_current_addr(asmd.orgs_filled - 1) - 1;
    switch (all_instructions[opcode].operands[k]) {
      case op_offset:
        asmd.subs[asmd.subs_filled].mode_and_id = 0x4000 + label_id;
        break;
      case op_addr16:
        asmd.subs[asmd.subs_filled].mode_and_id = 0x8000 + label_id;
        asmd.addr++;  // reserve next space for a byte
        break;
      case op_addr11:
        asmd.subs[asmd.subs_filled].mode_and_id = 0xc000 + label_id;
        break;
      default:
        break;
    }
    asmd.subs_filled++;
    check_subs_bounds();
  }

  if (debug) {
    print_instruction(debug_line, mnemonic, operands, mn, op, opcode, data);
  }
}

// substitute undefined data(labels) with offset or address
void substitute_labels() {
  for (int i = 0; i < asmd.subs_filled; i++) {
    char type = asmd.subs[i].mode_and_id >> 14;
    int label_id = asmd.subs[i].mode_and_id & 0x3fff;
    int hex_id = asmd.subs[i].hex_addr;
    int instr_addr = asmd.subs[i].addr;

    char *name = asmd.labels[label_id].name;
    int label_addr = asmd.labels[label_id].addr;

    if (label_addr == 0xffff) {
      print_error(0, "Label not defined", name);
    }

    switch (type) {
      case 1:; // offset
        int offset = label_addr - instr_addr;
        if (offset > 127 || offset < -128) {
          print_error(0, "Label offset out of range", name);
        }
        asmd.hex[hex_id] = offset < 0 ? 255 + offset : offset - 1;
        break;
      case 2: // addr16
        asmd.hex[hex_id] = label_addr >> 8;
        asmd.hex[hex_id + 1] = label_addr & 0x00ff;
        break;
      case 3: // addr11
        if (!set_addr11(&asmd.hex[hex_id - 1], &asmd.hex[hex_id], label_addr, instr_addr)) {
          print_error(0, "Label is not in the same page for ACALL", name);
        }
      default:
        break;
    }
  }
}

