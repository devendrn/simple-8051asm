#include "literals.h"

// all sfrs
const struct sfr sfr_bit_addressable[] = {
  {"acc",  0xe0},
  {"psw",  0xd0},
  {"p0",   0x80},
  {"p1",   0x90},
  {"p2",   0xa0},
  {"p3",   0xb0},
  {"ie",   0xa8},
  {"ip",   0xb8},
  {"tcon", 0x88},
  {"scon", 0x98},
  {"sbuf", 0x99}
};
const struct sfr sfr_non_bit_addressable[] = {
  {"sp",   0x81},
  {"dpl",  0x82},
  {"dph",  0x83},
  {"tl0",  0x8a},
  {"tl1",  0x8b},
  {"th0",  0x8c},
  {"th1",  0x8d},
  {"pcon", 0x87},
  {"tmod", 0x89},
  {"b",    0xf0}
};
const struct sfr sfr_bit[] = {
  // psw bits
  {"p",   0xd0},
  {"ov",  0xd2},
  {"rs0", 0xd3},
  {"rs1", 0xd4},
  {"f0",  0xd5},
  {"ac",  0xd6},
  {"cy",  0xd7},

  // tcon bits
  {"it0", 0x88},
  {"ie0", 0x89},
  {"it1", 0x8a},
  {"ie1", 0x8b},
  {"tr0", 0x8c},
  {"tf0", 0x8d},
  {"tr1", 0x8e},
  {"tf1", 0x8f},

  // scon bits
  {"ri",  0x98},
  {"ti",  0x99},
  {"rb8", 0x9a},
  {"tb8", 0x9b},
  {"ren", 0x9c},
  {"sm2", 0x9d},
  {"sm1", 0x9e},
  {"sm0", 0x9f},

  // ie bits
  {"ex0", 0xa8},
  {"et0", 0xa9},
  {"ex1", 0xaa},
  {"et1", 0xab},
  {"es",  0xac},
  {"ea",  0xaf},

  // ip bits
  {"px0", 0xb8},
  {"pt0", 0xb9},
  {"px1", 0xba},
  {"pt1", 0xbb},
  {"ps",  0xbc}
};

const char sfr_bit_n = sizeof(sfr_bit)/sizeof(struct sfr);
const char sfr_non_bit_addressable_n = sizeof(sfr_non_bit_addressable)/sizeof(struct sfr);
const char sfr_bit_addressable_n = sizeof(sfr_bit_addressable)/sizeof(struct sfr);

const char * all_operands[] = {
  "a",
  "@r0",
  "@r1",
  "r0",
  "r1",
  "r2",
  "r3",
  "r4",
  "r5",
  "r6",
  "r7",
  "c",
  "@a+dptr",
  "ab",
  "@a+pc",
  "dptr",
  "@dptr",
  ""
};

const char * all_mnemonics[] = {
  // 8051 mnemonics
  "nop",
  "ajmp",
  "ljmp",
  "rr",
  "inc",
  "jbc",
  "acall",
  "lcall",
  "rrc",
  "dec",
  "jb",
  "ret",
  "rl",
  "add",
  "jnb",
  "reti",
  "rlc",
  "addc",
  "jc",
  "orl",
  "jnc",
  "anl",
  "jz",
  "xrl",
  "jnz",
  "jmp",
  "mov",
  "sjmp",
  "movc",
  "div",
  "subb",
  "mul",
  "cpl",
  "cjne",
  "push",
  "clr",
  "swap",
  "xch",
  "pop",
  "setb",
  "da",
  "djnz",
  "xchd",
  "movx",

  // assembler mnemonics
  "",
  "org",
  "db"
};

// 8051 instruction table
const struct instruction all_instructions[256] = {
  {mn_nop,   {op_none, op_none, op_none}},
  {mn_ajmp,  {op_addr11, op_none, op_none}},
  {mn_ljmp,  {op_addr16, op_none, op_none}},
  {mn_rr,    {op_a, op_none, op_none}},
  {mn_inc,   {op_a, op_none, op_none}},
  {mn_inc,   {op_direct, op_none, op_none}},
  {mn_inc,   {op_ar0, op_none, op_none}},
  {mn_inc,   {op_ar1, op_none, op_none}},
  {mn_inc,   {op_r0, op_none, op_none}},
  {mn_inc,   {op_r1, op_none, op_none}},
  {mn_inc,   {op_r2, op_none, op_none}},
  {mn_inc,   {op_r3, op_none, op_none}},
  {mn_inc,   {op_r4, op_none, op_none}},
  {mn_inc,   {op_r5, op_none, op_none}},
  {mn_inc,   {op_r6, op_none, op_none}},
  {mn_inc,   {op_r7, op_none, op_none}},
  {mn_jbc,   {op_bit, op_offset, op_none}},
  {mn_acall, {op_addr11, op_none, op_none}},
  {mn_lcall, {op_addr16, op_none, op_none}},
  {mn_rrc,   {op_a, op_none, op_none}},
  {mn_dec,   {op_a, op_none, op_none}},
  {mn_dec,   {op_direct, op_none, op_none}},
  {mn_dec,   {op_ar0, op_none, op_none}},
  {mn_dec,   {op_ar1, op_none, op_none}},
  {mn_dec,   {op_r0, op_none, op_none}},
  {mn_dec,   {op_r1, op_none, op_none}},
  {mn_dec,   {op_r2, op_none, op_none}},
  {mn_dec,   {op_r3, op_none, op_none}},
  {mn_dec,   {op_r4, op_none, op_none}},
  {mn_dec,   {op_r5, op_none, op_none}},
  {mn_dec,   {op_r6, op_none, op_none}},
  {mn_dec,   {op_r7, op_none, op_none}},
  {mn_jb,    {op_bit, op_offset, op_none}},
  {mn_ajmp,  {op_addr11, op_none, op_none}},
  {mn_ret,   {op_none, op_none, op_none}},
  {mn_rl,    {op_a, op_none, op_none}},
  {mn_add,   {op_a, op_immed, op_none}},
  {mn_add,   {op_a, op_direct, op_none}},
  {mn_add,   {op_a, op_ar0, op_none}},
  {mn_add,   {op_a, op_ar1, op_none}},
  {mn_add,   {op_a, op_r0, op_none}},
  {mn_add,   {op_a, op_r1, op_none}},
  {mn_add,   {op_a, op_r2, op_none}},
  {mn_add,   {op_a, op_r3, op_none}},
  {mn_add,   {op_a, op_r4, op_none}},
  {mn_add,   {op_a, op_r5, op_none}},
  {mn_add,   {op_a, op_r6, op_none}},
  {mn_add,   {op_a, op_r7, op_none}},
  {mn_jnb,   {op_bit, op_offset, op_none}},
  {mn_acall, {op_addr11, op_none, op_none}},
  {mn_reti,  {op_none, op_none, op_none}},
  {mn_rlc,   {op_a, op_none, op_none}},
  {mn_addc,  {op_a, op_immed, op_none}},
  {mn_addc,  {op_a, op_direct, op_none}},
  {mn_addc,  {op_a, op_ar0, op_none}},
  {mn_addc,  {op_a, op_ar1, op_none}},
  {mn_addc,  {op_a, op_r0, op_none}},
  {mn_addc,  {op_a, op_r1, op_none}},
  {mn_addc,  {op_a, op_r2, op_none}},
  {mn_addc,  {op_a, op_r3, op_none}},
  {mn_addc,  {op_a, op_r4, op_none}},
  {mn_addc,  {op_a, op_r5, op_none}},
  {mn_addc,  {op_a, op_r6, op_none}},
  {mn_addc,  {op_a, op_r7, op_none}},
  {mn_jc,    {op_offset, op_none, op_none}},
  {mn_ajmp,  {op_addr11, op_none, op_none}},
  {mn_orl,   {op_direct, op_a, op_none}},
  {mn_orl,   {op_direct, op_immed, op_none}},
  {mn_orl,   {op_a, op_immed, op_none}},
  {mn_orl,   {op_a, op_direct, op_none}},
  {mn_orl,   {op_a, op_ar0, op_none}},
  {mn_orl,   {op_a, op_ar1, op_none}},
  {mn_orl,   {op_a, op_r0, op_none}},
  {mn_orl,   {op_a, op_r1, op_none}},
  {mn_orl,   {op_a, op_r2, op_none}},
  {mn_orl,   {op_a, op_r3, op_none}},
  {mn_orl,   {op_a, op_r4, op_none}},
  {mn_orl,   {op_a, op_r5, op_none}},
  {mn_orl,   {op_a, op_r6, op_none}},
  {mn_orl,   {op_a, op_r7, op_none}},
  {mn_jnc,   {op_offset, op_none, op_none}},
  {mn_acall, {op_addr11, op_none, op_none}},
  {mn_anl,   {op_direct, op_a, op_none}},
  {mn_anl,   {op_direct, op_immed, op_none}},
  {mn_anl,   {op_a, op_immed, op_none}},
  {mn_anl,   {op_a, op_direct, op_none}},
  {mn_anl,   {op_a, op_ar0, op_none}},
  {mn_anl,   {op_a, op_ar1, op_none}},
  {mn_anl,   {op_a, op_r0, op_none}},
  {mn_anl,   {op_a, op_r1, op_none}},
  {mn_anl,   {op_a, op_r2, op_none}},
  {mn_anl,   {op_a, op_r3, op_none}},
  {mn_anl,   {op_a, op_r4, op_none}},
  {mn_anl,   {op_a, op_r5, op_none}},
  {mn_anl,   {op_a, op_r6, op_none}},
  {mn_anl,   {op_a, op_r7, op_none}},
  {mn_jz,    {op_offset, op_none, op_none}},
  {mn_ajmp,  {op_addr11, op_none, op_none}},
  {mn_xrl,   {op_direct, op_a, op_none}},
  {mn_xrl,   {op_direct, op_immed, op_none}},
  {mn_xrl,   {op_a, op_immed, op_none}},
  {mn_xrl,   {op_a, op_direct, op_none}},
  {mn_xrl,   {op_a, op_ar0, op_none}},
  {mn_xrl,   {op_a, op_ar1, op_none}},
  {mn_xrl,   {op_a, op_r0, op_none}},
  {mn_xrl,   {op_a, op_r1, op_none}},
  {mn_xrl,   {op_a, op_r2, op_none}},
  {mn_xrl,   {op_a, op_r3, op_none}},
  {mn_xrl,   {op_a, op_r4, op_none}},
  {mn_xrl,   {op_a, op_r5, op_none}},
  {mn_xrl,   {op_a, op_r6, op_none}},
  {mn_xrl,   {op_a, op_r7, op_none}},
  {mn_jnz,   {op_offset, op_none, op_none}},
  {mn_acall, {op_addr11, op_none, op_none}},
  {mn_orl,   {op_c, op_bit, op_none}},
  {mn_jmp,   {op_aa_dptr, op_none, op_none}},
  {mn_mov,   {op_a, op_immed, op_none}},
  {mn_mov,   {op_direct, op_immed, op_none}},
  {mn_mov,   {op_ar0, op_immed, op_none}},
  {mn_mov,   {op_ar1, op_immed, op_none}},
  {mn_mov,   {op_r0, op_immed, op_none}},
  {mn_mov,   {op_r1, op_immed, op_none}},
  {mn_mov,   {op_r2, op_immed, op_none}},
  {mn_mov,   {op_r3, op_immed, op_none}},
  {mn_mov,   {op_r4, op_immed, op_none}},
  {mn_mov,   {op_r5, op_immed, op_none}},
  {mn_mov,   {op_r6, op_immed, op_none}},
  {mn_mov,   {op_r7, op_immed, op_none}},
  {mn_sjmp,  {op_offset, op_none, op_none}},
  {mn_ajmp,  {op_addr11, op_none, op_none}},
  {mn_anl,   {op_c, op_bit, op_none}},
  {mn_movc,  {op_a, op_aa_pc, op_none}},
  {mn_div,   {op_ab, op_none, op_none}},
  {mn_mov,   {op_direct, op_direct, op_none}},
  {mn_mov,   {op_direct, op_ar0, op_none}},
  {mn_mov,   {op_direct, op_ar1, op_none}},
  {mn_mov,   {op_direct, op_r0, op_none}},
  {mn_mov,   {op_direct, op_r1, op_none}},
  {mn_mov,   {op_direct, op_r2, op_none}},
  {mn_mov,   {op_direct, op_r3, op_none}},
  {mn_mov,   {op_direct, op_r4, op_none}},
  {mn_mov,   {op_direct, op_r5, op_none}},
  {mn_mov,   {op_direct, op_r6, op_none}},
  {mn_mov,   {op_direct, op_r7, op_none}},
  {mn_mov,   {op_dptr, op_immed, op_none}},
  {mn_acall, {op_addr11, op_none, op_none}},
  {mn_mov,   {op_bit, op_c, op_none}},
  {mn_movc,  {op_a, op_aa_dptr, op_none}},
  {mn_subb,  {op_a, op_immed, op_none}},
  {mn_subb,  {op_a, op_direct, op_none}},
  {mn_subb,  {op_a, op_ar0, op_none}},
  {mn_subb,  {op_a, op_ar1, op_none}},
  {mn_subb,  {op_a, op_r0, op_none}},
  {mn_subb,  {op_a, op_r1, op_none}},
  {mn_subb,  {op_a, op_r2, op_none}},
  {mn_subb,  {op_a, op_r3, op_none}},
  {mn_subb,  {op_a, op_r4, op_none}},
  {mn_subb,  {op_a, op_r5, op_none}},
  {mn_subb,  {op_a, op_r6, op_none}},
  {mn_subb,  {op_a, op_r7, op_none}},
  {mn_orl,   {op_c, op_bit, op_none}},
  {mn_ajmp,  {op_addr11, op_none, op_none}},
  {mn_mov,   {op_c, op_bit, op_none}},
  {mn_inc,   {op_dptr, op_none, op_none}},
  {mn_mul,   {op_ab, op_none, op_none}},
  {mn_none,  {op_none, op_none, op_none}},  // reserved 0xA5
  {mn_mov,   {op_ar0, op_direct, op_none}},
  {mn_mov,   {op_ar1, op_direct, op_none}},
  {mn_mov,   {op_r0, op_direct, op_none}},
  {mn_mov,   {op_r1, op_direct, op_none}},
  {mn_mov,   {op_r2, op_direct, op_none}},
  {mn_mov,   {op_r3, op_direct, op_none}},
  {mn_mov,   {op_r4, op_direct, op_none}},
  {mn_mov,   {op_r5, op_direct, op_none}},
  {mn_mov,   {op_r6, op_direct, op_none}},
  {mn_mov,   {op_r7, op_direct, op_none}},
  {mn_anl,   {op_c, op_bit, op_none}},
  {mn_acall, {op_addr11, op_none, op_none}},
  {mn_cpl,   {op_bit, op_none, op_none}},
  {mn_cpl,   {op_c, op_none, op_none}},
  {mn_cjne,  {op_a, op_immed, op_offset}},
  {mn_cjne,  {op_a, op_direct, op_offset}},
  {mn_cjne,  {op_ar0, op_immed, op_offset}},
  {mn_cjne,  {op_ar1, op_immed, op_offset}},
  {mn_cjne,  {op_r0, op_immed, op_offset}},
  {mn_cjne,  {op_r1, op_immed, op_offset}},
  {mn_cjne,  {op_r2, op_immed, op_offset}},
  {mn_cjne,  {op_r3, op_immed, op_offset}},
  {mn_cjne,  {op_r4, op_immed, op_offset}},
  {mn_cjne,  {op_r5, op_immed, op_offset}},
  {mn_cjne,  {op_r6, op_immed, op_offset}},
  {mn_cjne,  {op_r7, op_immed, op_offset}},
  {mn_push,  {op_direct, op_none, op_none}},
  {mn_ajmp,  {op_addr11, op_none, op_none}},
  {mn_clr,   {op_bit, op_none, op_none}},
  {mn_clr,   {op_c, op_none, op_none}},
  {mn_swap,  {op_a, op_none, op_none}},
  {mn_xch,   {op_a, op_direct, op_none}},
  {mn_xch,   {op_a, op_ar0, op_none}},
  {mn_xch,   {op_a, op_ar1, op_none}},
  {mn_xch,   {op_a, op_r0, op_none}},
  {mn_xch,   {op_a, op_r1, op_none}},
  {mn_xch,   {op_a, op_r2, op_none}},
  {mn_xch,   {op_a, op_r3, op_none}},
  {mn_xch,   {op_a, op_r4, op_none}},
  {mn_xch,   {op_a, op_r5, op_none}},
  {mn_xch,   {op_a, op_r6, op_none}},
  {mn_xch,   {op_a, op_r7, op_none}},
  {mn_pop,   {op_direct, op_none, op_none}},
  {mn_acall, {op_addr11, op_none, op_none}},
  {mn_setb,  {op_bit, op_none, op_none}},
  {mn_setb,  {op_c, op_none, op_none}},
  {mn_da,    {op_a, op_none, op_none}},
  {mn_djnz,  {op_direct, op_offset, op_none}},
  {mn_xchd,  {op_a, op_ar0, op_none}},
  {mn_xchd,  {op_a, op_ar1, op_none}},
  {mn_djnz,  {op_r0, op_offset, op_none}},
  {mn_djnz,  {op_r1, op_offset, op_none}},
  {mn_djnz,  {op_r2, op_offset, op_none}},
  {mn_djnz,  {op_r3, op_offset, op_none}},
  {mn_djnz,  {op_r4, op_offset, op_none}},
  {mn_djnz,  {op_r5, op_offset, op_none}},
  {mn_djnz,  {op_r6, op_offset, op_none}},
  {mn_djnz,  {op_r7, op_offset, op_none}},
  {mn_movx,  {op_a, op_adptr, op_none}},
  {mn_ajmp,  {op_addr11, op_none, op_none}},
  {mn_movx,  {op_a, op_ar0, op_none}},
  {mn_movx,  {op_a, op_ar1, op_none}},
  {mn_clr,   {op_a, op_none, op_none}},
  {mn_mov,   {op_a, op_direct, op_none}},
  {mn_mov,   {op_a, op_ar0, op_none}},
  {mn_mov,   {op_a, op_ar1, op_none}},
  {mn_mov,   {op_a, op_r0, op_none}},
  {mn_mov,   {op_a, op_r1, op_none}},
  {mn_mov,   {op_a, op_r2, op_none}},
  {mn_mov,   {op_a, op_r3, op_none}},
  {mn_mov,   {op_a, op_r4, op_none}},
  {mn_mov,   {op_a, op_r5, op_none}},
  {mn_mov,   {op_a, op_r6, op_none}},
  {mn_mov,   {op_a, op_r7, op_none}},
  {mn_movx,  {op_adptr, op_a, op_none}},
  {mn_acall, {op_addr11, op_none, op_none}},
  {mn_movx,  {op_ar0, op_a, op_none}},
  {mn_movx,  {op_ar1, op_a, op_none}},
  {mn_cpl,   {op_a, op_none, op_none}},
  {mn_mov,   {op_direct, op_a, op_none}},
  {mn_mov,   {op_ar0, op_a, op_none}},
  {mn_mov,   {op_ar1, op_a, op_none}},
  {mn_mov,   {op_r0, op_a, op_none}},
  {mn_mov,   {op_r1, op_a, op_none}},
  {mn_mov,   {op_r2, op_a, op_none}},
  {mn_mov,   {op_r3, op_a, op_none}},
  {mn_mov,   {op_r4, op_a, op_none}},
  {mn_mov,   {op_r5, op_a, op_none}},
  {mn_mov,   {op_r6, op_a, op_none}},
  {mn_mov,   {op_r7, op_a, op_none}}
};
