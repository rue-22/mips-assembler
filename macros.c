#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int gcd(int a, int b) {
    if(b == 0) {
        return a;
    } else if(b > 0) {
        int rem = (int) remainder(a, b);
        gcd(b, rem);
    }
}


void print_integer(int a) {
    printf("%d", instr.macro_inp_1);
    curr_pc += 12;
}


void read_integer(int a) {
    RF[24] = instr.macro_inp_1;
    curr_pc += 12;

}


void read_str(int i) {
    InstrDict = instr_list[i]; 
    char *buffer = malloc(instr.macro_inp_2);
    fgets(buffer, instr.macro_inp_2, stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
    for(int j = 0; j < n; j++) {
        if(instr_list[j].data_symbol != NULL && strcmp(instr_list[j].data_symbol, instr.data_symbol) == 0) {
            instr_list[j].asciiz_value = strdup(buffer); 
        }
    }
    Node *curr = head;
    int idx = find_position(instr.data_loc);
    for(int i = 0; i < idx; i++) {
        curr = curr->next;
    }
    curr->string = strdup(buffer);
    free(buffer);
    curr_pc += 20;
}


void print_str(char *string) {
    if(strcmp(instr_list[i].asciiz_value, "\\n\n") == 0) {
        printf("\n");
    }
    else printf("%s", instr_list[i].asciiz_value);
    curr_pc += 16;
}


void exit_program() {
    exit(0);
}