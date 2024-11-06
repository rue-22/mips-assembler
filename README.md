# MIPS assembler

This is a two-pass assembler built to convert corresponding **MIPS** code into an object file using **C**.

## Features

This MIPS assembler can convert the following instructions:

| R-type | I-type  | J-type | Branches | Pseudoinstructions | Syscalls           |
| ------ | ------- | ------ | -------- | ------------------ | ------------------ |
| `add`  | `addi`  | `jal`  | `beq`    | `move`             | #1: `print int`    |
| `sub`  | `addiu` | `jr`   | `bne`    | `li`               | #4: `print string` |
| `and`  | `lw`    |        |          | `la`               | #5: `read int`     |
| `or`   | `sw`    |        |          | `lw <label>`       | #8: `read string`  |
| `slt`  | `lui`   |        |          |                    | #10: `exit`        |
| `jr`   | `ori`   |        |          |                    | #11: `print char`  |

It also comes with different macros which can be called. The following macros included are the following:

-   `exit()`
-   `read_integer(int)`
-   `print_integer(int)`
-   `read_str(label, int)`
-   `print_str(label)`
-   `gcd(int, int)`

## Usage

### Compilation

To compile the assembler, run the command on your preferred shell:

`gcc -o mips_assembler cs21assembler.c && ./mips_assembler`

This implementation can be used by strictly following the specifications of the input:

### Input Format

-   The input must be a `.txt` file.
-   The first line of input must be an integer _n_ corresponding to the total number of words in the MIPS program.
-   There should be no spaces between commas. There should only be spaces between the label, mnemonic, and the operands.

### Output Format

1. A file containing all the symbols and their corresponding addresses generated during the first-pass of the assembler. This is saved on `symboltable.txt`.
2. A generated machine code (in binary) equivalent to the assembly instructions of
   the input text file. This is saved on `execute.txt`
3. Printing to the console the expected output of the MIPS program
   (if applicable).

## Authors

-   Salces, Carl John S.
-   Villamil, John Ysaac.
