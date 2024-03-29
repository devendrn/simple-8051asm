#ifndef LITERALS_H
#define LITERALS_H

// operands
// @ replaced by a, + replaced by _
enum operand_type {
  op_a,
  op_ar0,
  op_ar1,
  op_r0,
  op_r1,
  op_r2,
  op_r3,
  op_r4,
  op_r5,
  op_r6,
  op_r7,
  op_c,
  op_aa_dptr,
  op_ab,
  op_aa_pc,
  op_dptr,
  op_adptr,
  op_none,
  op_addr11,
  op_addr16,
  op_offset,
  op_bit,
  op_direct,
  op_immed,
  op_label,
  op_invalid
};

// mnemonics
enum mnemonic_type {
  // 8051 mnemonics
  mn_nop,
  mn_ajmp,
  mn_ljmp,
  mn_rr,
  mn_inc,
  mn_jbc,
  mn_acall,
  mn_lcall,
  mn_rrc,
  mn_dec,
  mn_jb,
  mn_ret,
  mn_rl,
  mn_add,
  mn_jnb,
  mn_reti,
  mn_rlc,
  mn_addc,
  mn_jc,
  mn_orl,
  mn_jnc,
  mn_anl,
  mn_jz,
  mn_xrl,
  mn_jnz,
  mn_jmp,
  mn_mov,
  mn_sjmp,
  mn_movc,
  mn_div,
  mn_subb,
  mn_mul,
  mn_cpl,
  mn_cjne,
  mn_push,
  mn_clr,
  mn_swap,
  mn_xch,
  mn_pop,
  mn_setb,
  mn_da,
  mn_djnz,
  mn_xchd,
  mn_movx,

  // assembler mnemonic
  mn_none,
  mn_org,
  mn_db,
  mn_invalid
};

// operand type and their value
struct operand {
  enum operand_type type;
  int value;
};

// label name and address (16 bit)
struct label {
  char name[16];
  int addr;
};

// special function registers
struct sfr {
  char name[5];
  int addr;
};

// all valid operands and mnemonics
extern const char* all_operands[];
extern const char* all_mnemonics[];

// all sfrs
extern const struct sfr sfr_bit[];
extern const struct sfr sfr_bit_addressable[];
extern const struct sfr sfr_non_bit_addressable[];
extern const char sfr_bit_n;
extern const char sfr_bit_addressable_n;
extern const char sfr_non_bit_addressable_n;

// all 8051 instructions
extern const struct instruction {
  enum mnemonic_type mnemonic;
  enum operand_type operands[3];
} all_instructions[];

#endif
