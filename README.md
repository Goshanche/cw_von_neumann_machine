# Simplified CISC Computer Architecture --- Assembler & Simulator

This project implements a simplified computer (SC) based on the
classical **von Neumann architecture**.\
It includes:

-   A **24-bit custom ISA**
-   A **two-pass assembler**
-   A **cycle-accurate instruction-level simulator**
-   An extended **instruction set** (arithmetic, logical, control,
    stack, and flag operations)
-   Support for **stack**, **ZF flag**, and **symbolic labels**

## Features

### Architecture Overview

-   **Data bus width:** 24 bits\
-   **Memory size:** 4096 bytes\
-   **Register file:** 8 general-purpose registers (`reg0–reg7`)\
-   **Stack:** 32 entries (32-bit words)\
-   **Status register:**
    -   `ZF` --- zero flag\
-   **Instruction formats:** R-type, I-type, J-type, O-type\
-   **Execution cycle:** fetch → decode → execute

# Instruction Formats

### R-Type

    23-21: opcode  
    20-18: regA  
    17-15: regB  
    14-3 : unused  
    2-0  : destReg

### I-Type

    23-21: opcode  
    20-18: regA  
    17-15: regB  
    14-0 : offset

### J-Type

    23-21: opcode  
    20-18: regA  
    17-15: regB  
    14-0 : unused

### O-Type

    23-21: opcode  
    20-0 : unused

# Instruction Set

## Standard Instructions

| Mnemonic             | Type | Description                        |
| -------------------- | ---- | ---------------------------------- |
| add regA regB dest   | R    | dest = regA + regB                 |
| nand regA regB dest  | R    | dest = ~(regA & regB)              |
| lw regA regB offset  | I    | regB = MEM[regA + offset]          |
| sw regA regB offset  | I    | MEM[regA + offset] = regB          |
| beq regA regB offset | I    | if (regA == regB) PC += offset + 1 |
| jalr regA regB       | R    | regB = PC; PC = regA               |
| halt                 | O    | Stop execution                     |
| noop                 | O    | No operation                       |
| .fill value          | —    | Store constant or label value      |


# Extended Instructions (Project-Specific)

## Arithmetic Instructions

-   dec regA --- regA = regA - 1
-   idiv --- Pop two stack values, push (a / b)
-   xadd regA regB dest --- dest = regA + regB; swap regA ↔ regB

## Logical Instructions

-   shl regA regB dest --- dest = regA \<\< regB
-   or regA regB dest --- dest = regA OR regB
-   neg --- Negates the top of the stack

## Control Instructions

-   jma regA regB offset --- if (regA \> regB) PC += offset + 1
    (unsigned)
-   jmne offset --- Pop two values; if (a != b) jump
-   je offset --- if (ZF == 1) PC += offset + 1
-   cmp regA regB --- ZF = (regA == regB)
-   bsr regA dest --- Scan regA for highest 1-bit → store position in
    dest

## Stack Instructions

-   push regA --- Push register value to stack
-   pop regA --- Pop top of stack into regA

# Assembler Description

The assembler uses a two-pass structure.

## Pass 1 --- Symbol Collection

-   Reads assembly file line-by-line
-   Extracts labels & validates instructions
-   Builds symbol table
-   Does not compute jump offsets yet

## Pass 2 --- Machine Code Generation

-   Re-reads the program
-   Encodes instructions into 24-bit numbers
-   Resolves labels and offsets
-   Verifies offset range (−16384 to +16384)
-   Writes machine code file

# Simulator Description

The simulator executes machine code at instruction-level precision.

### Execution Cycle

1.  Fetch instruction
2.  Decode
3.  Execute
4.  Write-back
5.  Update PC

### Simulator State

-   pc\
-   registers\
-   memory\
-   stack & sp\
-   zero flag (ZF)

Execution stops at `halt`.

# Example Programs

## 1. Division

    lw 0 1 a
    lw 0 2 b
    lw 0 3 c
    push 1
    push 2
    idiv
    pop 3
    halt

    a .fill 12
    b .fill 3
    c .fill 0

## 2. NEG + XADD

    lw 0 1 a
    lw 0 2 b
    lw 0 3 c
    push 1
    neg
    pop 1
    xadd 1 2 3
    halt

## 3. Conditional Jump

    lw 0 1 a
    lw 0 2 b
    lw 0 3 c
    cmp 1 2
    je two
    dec 2
    halt
    two or 1 2 3
    halt

# Build & Run

## Assemble

    gcc assembler.c -o assembler
    ./assembler program.asm program.mc

## Simulate

    gcc simulator.c -o simulator
    ./simulator program.mc

