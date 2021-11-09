# Processor

Welcome to the world of my very own tiny virtual CPU!

This project consists of 3 parts: an assembler, a disassembler and a processor.
Each part has its own executable, but really they are meant to be used together.

## Summary

* [Introduction](#introduction)
* [ASSembler](#assembler)
* [DisASSembler](#disassembler)
* [Processor](#processor)
* [Usage](#usage)
* [ASCII stuff](#ascii)

## ASSembler
The assembler program reads a text file written in a custom assembly language
and compiles it to a binary (compiled.jf by default) that can be then read by the CPU for execution.
  
Currently the following assembler commands are supported:
- __hlt__ - Ends program execution
- __ver__ - Verifies the integrity of the processor stack (only works if PROT_LEVEL > 0)
- __dmp__ - Dumps the info about the processor stack into a log file (only works if PROT_LEVEL > 0)
- __out__ - Pops a value off the stack and prints it to standard output
- __in__ - Reads a number from standard input and pushed it onto the stack
- __push 1__ - Pushes 1 (or any number) onto the stack
- __push ax__ - Pushes value from the __ax__ register onto the stack
- __push [1], push [ax], push [ax + 1]__ - Push values from CPU RAM onto the stack, where the RAM address is the total number value in the brackets
- __pop__ - Pops a value off the stack
- __pop ax__ - Pops a value from the stack into the __ax__ register
- __pop [1], ...__ - Pops the value from the stack into RAM
- __abs__ - Takes the absolute value of the top stack element, and pushes it onto the stack
- __sqrt__ - Takes the square root of the top stack element, and pushes it onto the stack
- __add__ - Pops 2 values off the stack, adds them, pushes them back onto the stack, right operand is the top stack value
- __sub__ - Adds 2 top stack values
- __mul__ - Multiplies 2 top stack values
- __div__ - Divides 2 top stack values
- __jmp lbl__ - Jumps to label __lbl__
- __ja lbl__  - Jumps to label __lbl__ if the top stack values satisfy the inequality __first > second__
- __jae lbl__ - Jumps if __>=__
- __jb lbl__ - Jumps if __<__
- __jbe lbl__ - Jumps if __<=__
- __je lbl__ - Jumps if __==__
- __jne lbl__ - Jumps if __!=__
- __call func__ - Jumps to a function with label __func__, the function must have a __ret__ in the end
- __draw__ - Draws lines of specified width from VRAM to standard output until \0 in is encountered
- __ret__ - Returns to the function call site

Labels are specified in the following form:
```
     jmp label
     ..
     some
     kode
     ..
     label:
        ..
        more
        kode
        ..  
```
          
The assembler ignores leading whitespaces and empty lines, so you can format your code to be more readable (just be careful not to add any extra spaces before and after command arguments or it won't compile)

You can find example .asm programs in the __examples__ folder of the repo

## DisASSembler 
The disassembler program reads a binary .jf file and decompiles it into the custom assembler program with using the command set described in the [ASSembler](#assembler) section
## Processor
The processor program takes in a compiled binary .jf file and executes the program from it. The processor version must match the command set version or the processor won't work.  

The processor has a dynamic stack for handling numeric values, 8 registers __ax - hx__ for storing values, a call stack that makes nested function calls possible and a 1 MB RAM (you can change its size by recompiling with a different __RAM_SIZE__ value)
## Usage
### Prerequisites
Unix based system or WSL(code written for Linux originally), __git(kinda)__, __gcc__, __make__  

You have to first compile the source code by using make in the root directory of the repo
```
git clone https://github.com/morgunovmi/processor && cd processor/

make && cd build/
```


Once you have written an assembler program you can compile it by running
```
./assembler program.asm
```
which will result in a __compiled.jf__ binary file. You can also explicitly name the binary file with the __-o__ flag
```
./assembler program.asm -o my_name_is.jf
```
By default this program also outputs a __clean.asm__ file, that contains your code without extra whitespaces. And obviously your program file has to be in the same directory as the executable.  

You can decompile a binary file by running
```
./disassembler compiled.jf
or
./disassembler compiled.jf -o bollocks.asm
```
You can run the compiled binary with the following command
```
./processor compiled.jf
```

## ASCII stuff

Don't forget to play around with the __ascii__ part of the __examples__.
For full support you need __ffmpeg__ and __jp2a__ programs:
```
sudo apt update
sudo apt install jp2a
sudo apt install ffmpeg
```
You can use the a2asm program to turn your image into a .gasm program, current
terminal dimensions are used for conversion
```
./a2asm img.jpeg
or explicitly
./a2asm -i img.jpeg
```
And finally, you can even convert your video into a .gasm program, so you
can enjoy your favourite show in ascii form in your terminal(without sound for now):
```
./a2asm -v video.mp4
```
This program will output a .gasm program that you can then compile and run!
![sus](https://github.com/morgunovmi/processor/blob/master/sus.png)

### GLHF!, @me with errors

