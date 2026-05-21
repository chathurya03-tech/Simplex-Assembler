# Simplex-Assembler and Emulator

Name: E V Sai Chathurya

# Description

Two-pass assembler and stack-based emulator for the SIMPLEX educational ISA. Built in C . It encodes/decodes 32-bit instructions, handles labels, branches, pseudo-instructions, and runtime safety checks.

# File Structure

. asm.c               # Two-pass assembler source
. emu.c               # Emulator source
. claims.txt          # Declaration of authorship
. bubble_sort.asm     # Bubble sort on 8-element array
. factorial.asm       # Iterative factorial (n=5)
. fibonacci.asm       # Iterative Fibonacci (N=10)
. sum.asm             # Sum of array elements
. triangle.asm        # Recursive triangle numbers
. test01.asm          # Valid program — unused label + infinite loop
. test02.asm          # Error-detection test (11 errors expected)
. test03.asm          # SET pseudo-instruction test
. *.o                 # Binary object files (assembler output)
. *.lst               # Listing files (assembler output)
. *.log               # Error/warning log files (assembler output)

# How to Build

Navigate to the project directory and run the following command to compile both the assembler and emulator

gcc asm.c -o assembler
gcc emu.c -o emulator

# How to Run

1. Assembler
To assemble a .asm file into a binary object .o file, generate a .lst listing file and also errors and warnings .log file:  ./asm <source.asm>
Produces three output files from source.asm

2.Emulator
To run the compiled binary .obj file through the virtual machine: ./emu <filename.o>
Loads the object file into memory and begins execution. On HALT, prints the final register state and a memory dump of all non-zero locations.
