# simple-8051asm

Simple 8051 Assembler is a simple, easy-to-use assembler for 8051 microcontrollers.
It is designed for students and hobbyists who want to quickly assemble small 8051 programs into a hex file.
The assembler is written in C using only standard libraries, and it packs output hex in Intel hex format (with a block size of 16).

You can test the output hex file with 8051 emulators like [emu8051](https://github.com/jarikomppa/emu8051).

## Accepted syntax

See [examples](examples/)
```
;comment
       mov a, #55
       mov r0, #-4
       add a, r0 ;another comment
       mov r1, a
       mov r2, 'g'
       mov p1, #0xff
here:  sjmp here
```

## Limits

There are some constraints to how this program works, and it is not production ready.
Use the output hex files at your own risk.
 - Labels longer than 16 characters are not allowed.
 - Max number of allowed labels is 512.
 - Can only assemble 2kb.
 - Call and Jump instructions must be explicit. Assembler does not substitute "call" and "jmp" keywords.

### To-do:

 - Auto call and jmp keyword
 - Dynamic allocation (fix label and hex limit)
 - Add more examples
 - Optimize code
 - More error checks
 - Input file validation

## Building

#### On Linux:

```
make
```

#### On Windows:

If you have gcc installed, open cmd in the project directory and run:
*not tested
```
gcc src/*.c -o asm51.exe
```

## Usage

simple-8051asm only accepts one input and output file.
```
asm51 [input_asm] -o [output_hex]

-h   help
-v   version
-o   output file
-d   debug
```
Example:
```
asm51 examples/sort.asm -o sort.hex
```
