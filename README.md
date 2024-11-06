# MIPS assembler

This is two-pass assembler built to convert corresponding **MIPS** code into an object file using **C**.

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

<!-- ### R-type

-   `add`
-   `sub`
-   `and`
-   `or`
-   `slt`
-   `jr`

### I-type

-   `addi`
-   `addiu`
-   `lw`
-   `sw`
-   `lui`
-   `ori`

### J-type

-   `jal`
-   `j`

### Branches

-   `beq`
-   `bne`

### Pseudoinstructions

-   `move`
-   `li`
-   `la`
-   `lw` -->
