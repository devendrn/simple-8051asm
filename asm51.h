#ifndef ASM51_HEADER
#define ASM51_HEADER

// assembler errors
enum asm_error {
	err_mnem,	// invalid mnemonic
	err_op,		// invalid operand
	err_instr,	// invalid instruction
	err_bou,	// label out of range
	err_dupli	// duplicate label
};

// @ replaced by a, + replaced by _
enum operand_type {
	op_a,op_ar0,op_ar1,op_r0,op_r1,op_r2,op_r3,op_r4,op_r5,op_r6,op_r7,op_c,
	op_aa_dptr,op_ab,op_aa_pc,op_dptr,op_adptr,op_none,op_addr11,op_addr16,
	op_offset,op_bit,op_direct,op_immed,op_label,op_invalid
};

enum mnemonic_type {
	mn_nop,mn_ajmp,mn_ljmp,mn_rr,mn_inc,mn_jbc,mn_acall,mn_lcall,mn_rrc,mn_dec,
	mn_jb,mn_ret,mn_rl,mn_add,mn_jnb,mn_reti,mn_rlc,mn_addc,mn_jc,mn_orl,mn_jnc,
	mn_anl,mn_jz,mn_xrl,mn_jnz,mn_jmp,mn_mov,mn_sjmp,mn_movc,mn_div,mn_subb,mn_mul,
	mn_cpl,mn_cjne,mn_push,mn_clr,mn_swap,mn_xch,mn_pop,mn_setb,mn_da,mn_djnz,
	mn_xchd,mn_movx,mn_none,mn_invalid
};

// operand type and their value
struct operand {
	enum operand_type type;
	int value;
};

// special function registers details
struct sfr {
	char name[4];
	int addr;
};

// label name and address (16 bit)
struct labels {
	char name[16];
	int addr;
};

struct instruction {
    enum mnemonic_type mnemonic;
    enum operand_type operands[3];
};

#endif
