# simple-8051asm

Simple 8051 Assembler is a simple, easy-to-use assembler for 8051 microcontrollers.
It is designed for students and hobbyists who want to quickly assemble small 8051 programs into a hex file.
The assembler is written in C using only standard libraries, and it packs output hex in Intel hex format (with a block size of 16).

You can test the output hex file with 8051 emulators like [emu8051](https://github.com/jarikomppa/emu8051).

## Manual

[Simple-8051asm Manual](manual.md)

### To-do:

 - Support for more assembly directives
 - Improve error warnings
 - Add more examples

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
