# Assembler features

## Instruction set

Simple-8051asm supports all 255 instructions of the 8051 microcontroller. Keywords are case insensitive. 

## SFR details

Only SFRs of Intel 8051 (MCS-51) architecture are defined.
```
SFRs: ACC, PSW, P0, P1, P2, P3, IE, IP, TCON, SCON, SBUF, SP, DPL, DPH, TL0, TL1, TH0, TH1, PCON, TMOD, B

SFR bits:
PSW  : P, OV, RS0, RS1, F0, AC, CY 
TCON : IT0, IE0, IT1, IE1, TR0, TF0, TR1, TF1
SC   : RI, TI, RB8, TB8, REN, SM2, SM1, SM0
IE   : EX0, ET0, EX1, ET1, ES, EA
IP   : PX0, PT0, PX1, PT1, PS
```

## Supported assembly directives:

- **ORG** command tells the assembler where to load the bytes into memory.  
  eg. `org 0x4110`

- **DB** command tells the assembler to load a data byte into memory.  
  eg. `db 0x2f`

## Value formats

Supported value formats are:
```
34    - Decimal
0xa3  - Hex
1011b - Binary
'a'   - ASCII
-5    - Signed decimal
```

## Comments

All characters after `;` are ignored in a line.

## Labels

- Should only consist of alphanumeric characters (`_` is allowed).
- Cannot have more than 16 characters.
- Cannot start with a number.
- Cannot be the same as a SFR, mnemonic, or operand keword.

## Example syntax

```
;comment
       mov a, #55
       mov r0, #-4
       add a, r0 ;another comment
       mov r1, a
       mov r2, 'g'
       mov p1, #0xff
here:  sjmp here
```
