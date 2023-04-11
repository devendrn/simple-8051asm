# simple-8051asm

Simple assembler for 8051 microcontrollers. Outputs in Intel Hex format

## !! Unfinished - Does not output any file !!
Only the parsing part is complete. Assembling part is incomplete.

## Building
#### On Linux:
```
cd simple-8051asm
gcc asm51.c -o asm51
```
## Usage
```
asm51 [input_asm] -o [output_hex]
```
Example
```
asm51 examples/sort.asm -o sort.hex
```
## Accepted syntax
Please see [examples](examples/). `START:` and `END` are not required inside the .asm file
```
;comment
       mov a,#55
       mov r0,#04
       add a,r0 ;another comment
       mov r1,a
here:  sjmp here
```
