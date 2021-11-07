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

## ASSembler
The assembler program reads a text file written in a custom assembly language
and compiles it to a binary that can be then read by the CPU for execution.\
  
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
- __ret__ - Returns to the function call site

Labels are specified in the following form:
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
          
The assembler ignores leading whitespaces and empty lines, so you can format your code to be more readable (just be careful not to add any extra spaces before and after command arguments or it won't compile)

## DisASSembler 
## Processor 
## Usage
