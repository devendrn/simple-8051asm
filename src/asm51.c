#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "literals.h"
#include "pack.h"
#include "debug.h"
#include "parser.h"
#include "assembler.h"

char mnemonic[16];
char operands[3][16];

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

  initialize_asmd();

  if (debug) {
    print_instruction_start();
  }

  while (!feof(asm_file)) {
    debug_line++;
    int current_addr = get_current_addr(asmd.orgs_filled - 1);
    if (parse_line(asm_file, mnemonic, operands, current_addr, asmd.labels)) {
      assemble(mnemonic, operands);
    }
    check_labels_bounds(); // to-do - move this
  }

  // end last org page and set hex filled size
  asmd.orgs[asmd.orgs_filled - 1][2] = asmd.addr - 1;
  asmd.hex_filled = asmd.addr;

  if (debug) {
    print_instruction_end();
    print_labels_table(asmd.labels);
    print_subs_table(asmd.subs, asmd.subs_filled);
    printf(" before substitute:");
    print_hex_array(asmd.hex, asmd.hex_filled);
  }

  substitute_labels();

  if (debug) {
    printf(" after substitute:");
    print_hex_array(asmd.hex, asmd.hex_filled);
    print_orgs_table(asmd.orgs, asmd.orgs_filled);
    print_packed_table(asmd.hex, asmd.orgs, asmd.orgs_filled);
  }

  fclose(asm_file);

  if (debug_errors == 0) {
    pack_ihex(file_out, asmd.hex, asmd.hex_filled, asmd.orgs, asmd.orgs_filled);
  }

  free(asmd.labels);
  free(asmd.hex);
  free(asmd.subs);
  free(asmd.orgs);

  return 0;
}
