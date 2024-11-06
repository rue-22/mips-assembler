#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
// #include "macros.c"

#define MAX_LEN 64 
#define MACHINE_LEN 34

typedef struct {
    char instr[MAX_LEN];
    char *funct_name;
    char *text_symbol;
    char *data_symbol;
    int text_loc;
    int data_loc;
    char *type;
    char *asciiz_value;
    int word_value;
    int macro_inp_1;
    int macro_inp_2;
    int bytes_space;
    char *rs;
    int rs_num;
    char *rt;
    int rt_num;
    char *rd;
    int rd_num;
    int imm;
    int jta;
    bool from_text_segment;
} InstrDict;

typedef struct {
    char *key;
    char *bin_code;
    char *type; 
    int reg_num;
} InstrDetails;

typedef struct Node {
    int mem_addr;
    char *datalabel;
    int data;    
    char *string;
    struct Node *next;
} Node;

InstrDetails LUT[] = { 
    {"li", "", "Ps", -1},
    {"lw", "100011", "", -1},
    {"la", "", "Ps", -1},
    {"add", "100000", "Ri", -1},
    {"sub", "100010", "Ri", -1},
    {"addi", "001000", "Ii", -1}, 
    {"addiu", "001001", "Ii", -1},
    {"and", "100100", "Ri", -1},
    {"or", "100101", "Ri", -1},
    {"slt", "101010", "Ri", -1}, 
    {"beq", "000100", "Bi", -1},
    {"bne", "000101", "Bi", -1},
    {"sw", "101011", "Ii", -1},
    {"move", "100000", "Ps", -1},
    {"j", "000010", "Ji", -1},
    {"jal", "000011", "Ji", -1},
    {"jr", "001000", "Ri", -1},
    {"lui", "001111", "Ii", -1},
    {"ori", "001101", "Ii", -1},
    {"syscall", "00000000000000000000000000000000", "", -1},
    {"$0", "00000", "", 0},
    {"$at", "00001", "", 1},
    {"$v0", "00010", "", 2},
    {"$v1", "00011", "", 3},
    {"$a0", "00100", "", 4},
    {"$a1", "00101", "", 5},
    {"$a2", "00110", "", 6},
    {"$a3", "00111", "", 7},
    {"$t0", "01000", "", 8},
    {"$t1", "01001", "", 9},
    {"$t2", "01010", "", 10},
    {"$t3", "01011", "", 11},
    {"$t4", "01100", "", 12},
    {"$t5", "01101", "", 13},
    {"$t6", "01110", "", 14},
    {"$t7", "01111", "", 15},
    {"$s0", "10000", "", 16},
    {"$s1", "10001", "", 17},
    {"$s2", "10010", "", 18},
    {"$s3", "10011", "", 19},
    {"$s4", "10100", "", 20},
    {"$s5", "10101", "", 21},
    {"$s6", "10110", "", 22},
    {"$s7", "10111", "", 23},
    {"$t8", "11000", "", 24},
    {"$t9", "11001", "", 25},
    {"$k0", "11010", "", 26},
    {"$k1", "11011", "", 27},
    {"$gp", "11100", "", 28},
    {"$sp", "11101", "", 29},
    {"$fp", "11110", "", 30},
    {"$ra", "11111", "", 31}
};


int RF[] = {0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 268468224, 2147479548, 0, 0};


int curr_pc = 0x00400000;


int find_index_in_lut(int, char *);
void convert_r_to_machine(FILE *, char *, char *, char *, char *, int);
void convert_i_to_machine(FILE *, char *, char *, char *, int, int);
void convert_j_to_machine(FILE *, char *, int, int);
void convert_ps_to_machine(FILE *, int);
void convert_macro_to_machine(FILE *, int);
bool execute(int);
void execute_syscall();
void execute_r(int);
void execute_i(int);
void execute_j(int);
void execute_branch(int);
void execute_pseudo(int);
void execute_macro(int);
int gcd(int, int);
void compute_jta(int, char *);
void get_first_two_op(int, char *, bool);
char *int_to_binary(int, int);
void assign_val_to_symbol(int, int, char *, char *);
bool check_duplicate_symbol(int, char *);
void write_to_symbol_table(FILE *, int);
void print_node(int);
void add_node(int, char *, int, char *);
int find_position(int);
void print_ll();


// GLOBAL instruction struct array
InstrDict *instr_list = NULL;
Node *head = NULL;
int n;

int main() {
    // use a list of structs to store instructions [{pc : instr}]
    FILE *fp; 
    // program counter (pc)
    int text_pc = 0x00400000;
    int data_pc = 0x10000000;

    // read inputs from text file
    fp = fopen("mips.txt", "r");
    char buffer[MAX_LEN];
    bool isFromText;
    while(fgets(buffer, MAX_LEN, fp)) {
        // remove trailing newline
        buffer[strcspn(buffer, "\n")] = 0;

        // if buffer is number, then it is the first input
        if(isdigit(buffer[0])) {
            n = atoi(buffer);
            instr_list = (InstrDict *) malloc(n * sizeof(InstrDict));
            // initialize all fields to default 
            for(int i = 0; i < n; i++) {
                instr_list[i].funct_name = NULL;
                instr_list[i].text_symbol = NULL;
                instr_list[i].data_symbol = NULL;
                instr_list[i].type = NULL;
                instr_list[i].asciiz_value = NULL;
                instr_list[i].rs = NULL;
                instr_list[i].rs_num = -1;
                instr_list[i].rt = NULL;
                instr_list[i].rt_num = -1;
                instr_list[i].rd = NULL;
                instr_list[i].rd_num = -1;
                instr_list[i].imm = 0;
                instr_list[i].jta = 0;
                instr_list[i].word_value = 0;
                instr_list[i].bytes_space = 0;
                instr_list[i].macro_inp_1 = 0;
                instr_list[i].macro_inp_2 = 0;
            }
            continue;
        } 

        // read instructions
        static int i = 0;
        if(i < n) {
            strncpy(instr_list[i].instr, buffer, MAX_LEN);
            // .include and .text segment
            if(buffer[0] == '.' && strcmp(buffer, ".data") != 0) {
                isFromText = true;
                instr_list[i].text_loc = INT_MAX; 
                instr_list[i].data_loc = INT_MAX;
                instr_list[i].from_text_segment  = false;
            }

            // .data segment
            else if(strcmp(buffer, ".data") == 0) {
                isFromText = false;
                instr_list[i].text_loc = INT_MAX;
                instr_list[i].data_loc = INT_MAX;
                instr_list[i].from_text_segment  = false;
            }

            // normal instruction
            else if(isFromText) {
                instr_list[i].from_text_segment  = true;
            } 
            else if(!isFromText) {
                instr_list[i].from_text_segment = false;
            }
            i++;
        }
    }
    fclose(fp);


    //* CHECKPOINT 1: Creating the symbol table 
    fp = fopen("symboltable.txt", "w");
    // separate the instructions using appropriate delimiters
    for(int i = 0; i < n; i++) {
        // parsing text segments
        if(instr_list[i].from_text_segment) {
            // make a copy so that it doesn't affect the original 
            char *instr_copy;

            // instruction is just normal instructions
            // split with spaces
            if(true) {
                instr_copy = strdup(instr_list[i].instr);
                char *token = strtok(instr_copy, " ");

                while(token != NULL) {
                    // label
                    if(strchr(token, ':') != NULL) {
                        instr_list[i].text_symbol = malloc(strlen(token) - 1);
                        strncpy(instr_list[i].text_symbol, token, strlen(token) - 1);
                        instr_list[i].text_symbol[strlen(token) - 1] = '\0';
                        goto next_token;
                    } 

                    if(strchr(token, '$') != NULL) {
                        goto next_token;
                    }

                    // function name
                    if(strchr(token, '(') == NULL) {
                        if(instr_list[i].funct_name == NULL) instr_list[i].funct_name = strdup(token);
                        int lut_len = sizeof(LUT)/sizeof(InstrDetails);
                        if(strcmp(instr_list[i].funct_name, "lw") != 0) {
                            for(int j = 0; j < lut_len; j++) {
                                if(strcmp(LUT[j].key, instr_list[i].funct_name) == 0) {
                                    instr_list[i].type = strdup(LUT[j].type);
                                    break;
                                }       
                            } 
                        }
                        goto next_token;
                    } 

                    next_token: token = strtok(NULL, " ");
                }
            }

            // instruction is lw, sw
            instr_copy = strdup(instr_list[i].instr);
            if(strstr(instr_copy, "lw") != NULL || strstr(instr_copy, "sw") != NULL) {
                char *token = strtok(instr_copy, ",");
                token = strtok(NULL, ",");

                // another symbol
                if(strchr(token, '$') == NULL) {
                    bool is_dupe_symbol = check_duplicate_symbol(i, token);
                    if(is_dupe_symbol) {
                        goto end_if2;
                    }
                    // copy symbol
                    instr_list[i].data_loc = data_pc;
                    instr_list[i].macro_inp_1 = data_pc;
                    data_pc += 4;


                    end_if2: instr_list[i].data_symbol = strdup(token);
                    instr_list[i].type = strdup("Ps");
                } else instr_list[i].type = strdup("Ii");
            }

            instr_copy = strdup(instr_list[i].instr);
            if(strstr(instr_copy, "la") != NULL) {
                char *token = strtok(instr_copy, ",");
                token = strtok(NULL, ",");

                bool is_dupe_symbol = check_duplicate_symbol(i, token);
                if(is_dupe_symbol) {
                    goto end_if3;
                }
                // copy symbol
                instr_list[i].data_loc = data_pc;
                instr_list[i].macro_inp_1 = data_pc;
                data_pc += 4;

                end_if3: instr_list[i].data_symbol = strdup(token);
                instr_list[i].type = strdup("Ps");
            }

            // instruction is print_int or read_int 
            instr_copy = strdup(instr_list[i].instr);
            if(strstr(instr_copy, "print_integer") || strstr(instr_copy, "read_integer")){
                char *token;
                if(strchr(instr_copy, ' ') != NULL) {
                    token = strtok(instr_copy, " "); 
                    token = strtok(NULL, "(");
                }
                else token = strtok(instr_copy, "(");
                instr_list[i].funct_name = strdup(token);
                token = strtok(NULL, ")");

                instr_list[i].type = strdup("Ma");
                instr_list[i].macro_inp_1 = atoi(token);
            }

            // instruction is print_str
            instr_copy = strdup(instr_list[i].instr);
            if(strstr(instr_copy, "print_str") != NULL) {

                char *token;
                if(strchr(instr_copy, ' ') != NULL) {
                    token = strtok(instr_copy, " "); 
                    token = strtok(NULL, "(");
                }
                else token = strtok(instr_copy, "(");
                instr_list[i].funct_name = strdup(token);
                token = strtok(NULL, ")");

                // copy symbol
                bool is_dupe_symbol = check_duplicate_symbol(i, token);
                if(is_dupe_symbol) {
                    goto end_if;
                };
                instr_list[i].data_symbol = strdup(token);
                instr_list[i].data_loc = data_pc;
                instr_list[i].macro_inp_1 = data_pc;
                data_pc += 4;


                end_if: instr_list[i].type = strdup("Ma");
                instr_list[i].data_symbol = strdup(token);
            }

            // instruction is read_str
            instr_copy = strdup(instr_list[i].instr);
            if(strstr(instr_copy, "read_str") != NULL) {
                char *token;
                if(strchr(instr_copy, ' ') != NULL) {
                    token = strtok(instr_copy, " "); 
                    token = strtok(NULL, "(");
                }
                else token = strtok(instr_copy, "(");
                instr_list[i].funct_name = strdup(token);
                token = strtok(NULL, ",");

                bool is_dupe_symbol = check_duplicate_symbol(i, token);
                if(is_dupe_symbol) {
                    instr_list[i].data_symbol = strdup(token);
                    goto end_if1;
                }

                // copy symbol and get first input argument
                instr_list[i].data_symbol = strdup(token);
                instr_list[i].data_loc = data_pc;
                instr_list[i].macro_inp_1 = data_pc;
                data_pc += 4;

                // get second input argument
                token = strtok(NULL, ")");
                instr_list[i].macro_inp_2 = atoi(token);

                end_if1: instr_list[i].type = strdup("Ma");
            }

            if(strcmp(instr_copy, "exit()") == 0) {
                char *token;
                if(strchr(instr_copy, ' ') != NULL) {
                    token = strtok(instr_copy, " "); 
                    token = strtok(NULL, "(");
                }
                else token = strtok(instr_copy, "(");
                instr_list[i].funct_name = strdup(token);
                instr_list[i].type = strdup("Ma");
            }
        }

        // data segment
        else {
            // skip directives
            if(strstr(instr_list[i].instr, ".text") != NULL || strstr(instr_list[i].instr, ".data") != NULL || strstr(instr_list[i].instr, ".include") != NULL) {
                continue;
            }

            char *instr_copy;
            InstrDict instr = instr_list[i];

            // allocate_str 
            instr_copy = strdup(instr_list[i].instr);
            if(strstr(instr_copy, "allocate_str") != NULL) {
                char *token = strtok(instr_copy, "(");
                token = strtok(NULL, ",");
                char *datalabel = strdup(token);

                token = strtok(NULL, "\"");
                assign_val_to_symbol(i, 0, token, datalabel);
                int data_loc = instr_list[i].data_loc;
                char *symbol = strdup(instr_list[i].data_symbol);
                int word_value = instr_list[i].word_value;
                char *asciiz_value = strdup(instr_list[i].asciiz_value);
                add_node(data_loc, symbol, word_value, asciiz_value);
                
                instr_copy = strdup(instr_list[i].instr);
            }                

            else if(strstr(instr_copy, "allocate_bytes") != NULL) {
                char *token = strtok(instr_copy, "(");
                token = strtok(NULL, ",");
                char *datalabel = strdup(token);

                token = strtok(NULL, ")");
                assign_val_to_symbol(i, atoi(token), NULL, datalabel);
                int data_loc = instr_list[i].data_loc;
                char *symbol = strdup(instr_list[i].data_symbol);
                int word_value = instr_list[i].word_value;
                char *asciiz_value = strdup(instr_list[i].asciiz_value);
                add_node(data_loc, symbol, word_value, asciiz_value);

                instr_copy = strdup(instr_list[i].instr);
            }

            // normal datalabel
            else if(strstr(instr_copy, "allocate") == NULL) {
                char *token = strtok(instr_copy, ": .");
                char *datalabel = strdup(token);

                instr_list[i].data_symbol = strdup(token);
                token = strtok(NULL, " ");

                // .word
                if(strstr(token, "word") != NULL) {
                    token = strtok(NULL, " ");
                    assign_val_to_symbol(i, atoi(token), NULL, datalabel);
                    int data_loc = instr_list[i].data_loc;
                    char *symbol = strdup(instr_list[i].data_symbol);
                    int word_value = instr_list[i].word_value;
                    char *asciiz_value = strdup(instr_list[i].asciiz_value);
                    add_node(data_loc, symbol, word_value, asciiz_value);
                }

                // .asciiz
                else if(strstr(token, "asciiz") != NULL) {
                    token = strtok(NULL, "\"");
                    char *tok_with_newline = malloc(strlen(token) + 2);
                    strcpy(tok_with_newline, token);
                    strcat(tok_with_newline, "\n");
                    assign_val_to_symbol(i, 0, tok_with_newline, datalabel);
                    char *symbol = strdup(instr_list[i].data_symbol);
                    int data_loc = instr_list[i].data_loc;
                    int word_value = instr_list[i].word_value;
                    char *asciiz_value = strdup(instr_list[i].asciiz_value);
                    int word_equiv = 0;
                    for(int i = strlen(token); i >= 0; i--) {
                        word_equiv = (word_equiv << 8) | token[i];
                    }
                    add_node(data_loc, symbol, word_equiv, asciiz_value);
                }
            }
        }
    }
    
    // assign addresses 
    for(int i = 0; i < n; i++) {
        if(instr_list[i].type != NULL && instr_list[i].from_text_segment){
            instr_list[i].text_loc = text_pc;
            if(strcmp(instr_list[i].type, "Ri") == 0 || strcmp(instr_list[i].type, "Ii") == 0 || strcmp(instr_list[i].type, "Ji") == 0 || strcmp(instr_list[i].type, "Bi") == 0) {
                text_pc += 4;
            }
            else if(strcmp(instr_list[i].type, "Ps") == 0) {
                if(strcmp(instr_list[i].funct_name, "la") == 0 || strcmp(instr_list[i].funct_name, "lw") == 0) text_pc += 8;
                else text_pc += 4;
            }
            else if(strcmp(instr_list[i].type, "Ma") == 0) {
                if(strcmp(instr_list[i].funct_name, "read_str") == 0) text_pc += 20;
                else if(strcmp(instr_list[i].funct_name, "print_str") == 0) text_pc += 16;
                else if(strcmp(instr_list[i].funct_name, "read_integer") == 0) text_pc += 12;
                else if(strcmp(instr_list[i].funct_name, "print_integer") == 0) text_pc += 12;
                else if(strcmp(instr_list[i].funct_name, "exit") == 0) text_pc += 8;
            }
            else {
                text_pc += 4;
            }
        }

        write_to_symbol_table(fp, i);
    }
    fclose(fp);

    // CHECKPOINT 2: Conversion and execution of instructions 
    for(int i = 0; i < n; i++) {
        // convert instructions only from text segment
        char *instr_copy = strdup(instr_list[i].instr);
        if(instr_list[i].from_text_segment) {
            char *token;
            if(instr_list[i].text_symbol != NULL && strcmp(instr_list[i].type, "Ma") != 0) {
                token = strtok(instr_copy, " ");
                token = strtok(NULL, " ");
                token = strtok(NULL, " ");
            } else if(instr_list[i].text_symbol == NULL && strcmp(instr_list[i].type, "Ma") != 0) {
                token = strtok(instr_copy, " ");
                token = strtok(NULL, " ");
            }

            // R-type instructions
            if(strcmp(instr_list[i].funct_name, "syscall") == 0) {
            }
            else if(strcmp(instr_list[i].funct_name, "jr") == 0) {
                instr_list[i].rs = strdup(token);
            }
            else if(strcmp(instr_list[i].type, "Ri") == 0) {
                token = strtok(token, ",");
                instr_list[i].rd = strdup(token); 
                token = strtok(NULL, ",");
                instr_list[i].rs = strdup(token);
                token = strtok(NULL, ",");
                instr_list[i].rt = strdup(token);
            }
            // I-type instructions
            else if(strcmp(instr_list[i].type, "Ii") == 0) {
                // loadword instruction
                if(strcmp(instr_list[i].funct_name, "lw") == 0 || strcmp(instr_list[i].funct_name, "sw") == 0) {
                    token = strtok(token, ",");
                    instr_list[i].rt = strdup(token);
                    token = strtok(NULL, "(");
                    instr_list[i].imm = atoi(token);
                    token = strtok(NULL, ")");
                    instr_list[i].rs = strdup(token);
                }
                else if(strcmp(instr_list[i].funct_name, "lui") == 0) {
                    token = strtok(token, ","); 
                    instr_list[i].rt = strdup(token);
                    token = strtok(NULL, ",");
                    instr_list[i].imm = atoi(token);
                }
                else get_first_two_op(i, token, false);
            }
            // Branch instructions
            else if(strcmp(instr_list[i].type, "Bi") == 0) {
                get_first_two_op(i, token, true);
            }
            // Jump instructions
            else if(strcmp(instr_list[i].type, "Ji") == 0) {
                if(strcmp(instr_list[i].instr, "j gcd") == 0) {
                }
                else if(strcmp(instr_list[i].instr, "jal gcd") == 0) {
                }
                else if(strcmp(instr_list[i].funct_name, "j") == 0) {
                    compute_jta(i, token);                    
                }
                else if(strcmp(instr_list[i].funct_name, "jal") == 0) {
                    compute_jta(i, token);
                }
            }
            // Pseudoinstruction
            else if(strcmp(instr_list[i].type, "Ps") == 0) {
                if(strcmp(instr_list[i].funct_name, "move") == 0) {
                    token = strtok(token, ",");
                    instr_list[i].rd = strdup(token);
                    int l = find_index_in_lut(i, instr_list[i].rd);
                    instr_list[i].rd_num = LUT[l].reg_num; 
                    token = strtok(NULL, ",");
                    instr_list[i].rs = strdup(token); 
                    l = find_index_in_lut(i, instr_list[i].rs);
                    instr_list[i].rs_num = LUT[l].reg_num; 
                }
                else if(strcmp(instr_list[i].funct_name, "li") == 0) {
                    token = strtok(token, ",");
                    instr_list[i].rt = strdup(token);
                    int l =find_index_in_lut(i, instr_list[i].rt);
                    instr_list[i].rt_num = LUT[l].reg_num; 
                    token = strtok(NULL, ",");
                    instr_list[i].imm = atoi(token); 
                }
                else if(strcmp(instr_list[i].funct_name, "lw") == 0 || strcmp(instr_list[i].funct_name, "la") == 0) {
                    token = strtok(token, ",");
                    instr_list[i].rt = strdup(token);
                    int l = find_index_in_lut(i, instr_list[i].rt);
                    instr_list[i].rt_num = LUT[l].reg_num; 
                }
            }
            // Macro
            else if(strcmp(instr_list[i].type, "Ma") == 0) {
            }
        }
        if (!instr_list[i].from_text_segment){
            char *token;
            if(strstr(instr_list[i].instr, "allocate_str") != NULL || strstr(instr_list[i].instr, "allocate_bytes") != NULL) {
                token = strtok(instr_copy, "(");
                token = strtok(NULL, ",");
                instr_list[i].data_symbol = strdup(token);
                bool flag = check_duplicate_symbol(i, token);
            }
            else if(strstr(instr_list[i].instr, ".asciiz") != NULL || strstr(instr_list[i].instr, ".word") != NULL) {
                token = strtok(instr_copy, ":");
                instr_list[i].data_symbol = strdup(token);
                bool flag = check_duplicate_symbol(i, token);
            }
        }
    }
    printf("Assemble: operation completed successfully.\n");

    // execution
    bool is_exit = false;
    fp = fopen("execute.txt", "w");
    while(is_exit == false && curr_pc < text_pc) {
        int i;
        for(int k = 0; k < n; k++) {
            if(instr_list[k].from_text_segment == true && instr_list[k].text_loc == curr_pc) {
                i = k;   
                break;
            }
        }

        char *instr_copy = strdup(instr_list[i].instr);
        if(instr_list[i].from_text_segment) {
            // R-type instructions
            if(strcmp(instr_list[i].funct_name, "syscall") == 0) {
                fprintf(fp, "00000000000000000000000000000000\n");
            }
            else if(strcmp(instr_list[i].funct_name, "jr") == 0) {
                convert_r_to_machine(fp, instr_list[i].funct_name, instr_list[i].rd, instr_list[i].rs, instr_list[i].rt, i);
            }
            else if(strcmp(instr_list[i].type, "Ri") == 0) {
                convert_r_to_machine(fp, instr_list[i].funct_name, instr_list[i].rd, instr_list[i].rs, instr_list[i].rt, i);
            
            }
            // I-type instructions
            else if(strcmp(instr_list[i].type, "Ii") == 0) {
                // loadword instruction
                convert_i_to_machine(fp, instr_list[i].funct_name, instr_list[i].rs, instr_list[i].rt, instr_list[i].imm, i);

            }
            // Branch instructions
            else if(strcmp(instr_list[i].type, "Bi") == 0) {
                convert_i_to_machine(fp, instr_list[i].funct_name, instr_list[i].rs, instr_list[i].rt, instr_list[i].imm, i);
            }
            // Jump instructions
            else if(strcmp(instr_list[i].type, "Ji") == 0) {
                if(strcmp(instr_list[i].instr, "j gcd") == 0) {
                    fprintf(fp, "00001000000000000100000000000000\n\0");
                }
                else if(strcmp(instr_list[i].instr, "jal gcd") == 0) {
                    fprintf(fp, "00001100000000000100000000000000\n\0");
                }
                else if(strcmp(instr_list[i].funct_name, "j") == 0) {
                    convert_j_to_machine(fp, instr_list[i].funct_name, instr_list[i].jta, i);
                }
                else if(strcmp(instr_list[i].funct_name, "jal") == 0) {
                    convert_j_to_machine(fp, instr_list[i].funct_name, instr_list[i].jta, i);
                }
            }
            // Pseudoinstruction
            else if(strcmp(instr_list[i].type, "Ps") == 0) {
                convert_ps_to_machine(fp, i);
            }
            // Macro
            else if(strcmp(instr_list[i].type, "Ma") == 0) {
                convert_macro_to_machine(fp, i);
            }
        }
        is_exit = execute(i);
    }
    fclose(fp);
}


int find_index_in_lut(int i, char *lookup_val) {
    int lut_len = sizeof(LUT)/sizeof(InstrDetails);
    for(int k = 0; k < lut_len; k++) {
        if(lookup_val != NULL && strcmp(LUT[k].key, lookup_val) == 0) {
            return k;
        }
    }
}


bool execute(int i) {
    InstrDict instr = instr_list[i];

    if(strcmp(instr.funct_name, "exit") == 0) { 
        exit(0);
    }
    else if(strcmp(instr.funct_name, "syscall") == 0) {
        curr_pc += 4;
        execute_syscall();
        return false;
    } 
    else if(strcmp(instr.type, "Ri") == 0) {
        execute_r(i);
        return false;
    }
    else if(strcmp(instr.type, "Ii") == 0) {
        execute_i(i);
        return false;
    }
    else if(strcmp(instr.type, "Ji") == 0) {
        execute_j(i);
        return false;
    }
    else if(strcmp(instr.type, "Bi") == 0) {
        execute_branch(i);
        return false;
    }
    else if(strcmp(instr.type, "Ps") == 0) {
        execute_pseudo(i);
        return false;
    }
    else if(strcmp(instr.type, "Ma") == 0) {
        execute_macro(i);
        return false;
    }
    else {
        curr_pc += 4;
        return false;
    }
}


void execute_syscall() {
    int v0 = RF[2];
    int a0 = RF[4];
    int a1 = RF[5];

    // print int
    if(v0 == 1){// int 
        printf("%d", a0);
    }
    // print string
    else if(v0 == 4){
        int idx = find_position(a0);
        print_node(idx);
    }
    // read int 
    else if(v0 == 5){
        scanf("%d", &v0);
    }
    // read string
    else if(v0 == 8){
        //fgets
        char *buffer = malloc(a1);
        fgets(buffer, a1, stdin);
    }
    // exit
    else if(v0 == 10){ 
        exit(0);
    }
    // print char
    else if(v0 == 11){
        printf("%c", a0);
    }
}


void execute_r(int i) {
    InstrDict instr = instr_list[i];
    int rs = instr.rs_num; 
    int rt = instr.rt_num;
    int rd = instr.rd_num;

    if(strcmp(instr.funct_name, "add") == 0) RF[rd] = RF[rs] + RF[rt];
    else if(strcmp(instr.funct_name, "sub") == 0) RF[rd] = RF[rs] - RF[rt]; 
    else if(strcmp(instr.funct_name, "and") == 0) RF[rd] = RF[rs] & RF[rt]; 
    else if(strcmp(instr.funct_name, "or") == 0) RF[rd] = RF[rs] | RF[rt]; 
    else if(strcmp(instr.funct_name, "slt") == 0) RF[rd] = (RF[rs] < RF[rt]) ? 1 : 0; 
    else if(strcmp(instr.funct_name, "jr") == 0) {
        curr_pc = RF[rs];
        return;
    }
    curr_pc += 4;
}


void execute_i(int i) {
    InstrDict instr = instr_list[i];
    int rs = instr.rs_num; 
    int rt = instr.rt_num;
    int imm = instr.imm;

    if(strcmp(instr.funct_name, "addi") == 0){
        RF[rt] = RF[rs] + imm; 
        if((imm > 0) && (RF[rs] > INT_MAX - imm)){
            printf("Overflow detected\n");
            exit(0);
        }
        else if((imm < 0) && (RF[rs] > INT_MAX + imm)){
            printf("Overflow detected\n");
            exit(0);
        }
    }
    else if(strcmp(instr.funct_name, "addiu") == 0) RF[rt] = RF[rs] + imm; 
    else if(strcmp(instr.funct_name, "ori") == 0) RF[rt] = RF[rs] | imm; 
    else if(strcmp(instr.funct_name, "sw") == 0) {
        int mem_addr = RF[rs] + instr.imm;
        if(mem_addr < 0x10000000) {
            printf("Invalid memory location to store to.\n");
            exit(0);
        }
        int idx = find_position(mem_addr);
        if(idx == -1) add_node(mem_addr, NULL, RF[rt], NULL);
        else {
            Node *curr = head;
            for(int i = 0; i < idx; i++) {
                curr = curr->next;
            }
            curr->data = RF[rt];
        }
    }
    else if(strcmp(instr.funct_name, "lw") == 0) {
        int mem_addr = RF[rs] + instr.imm; 
        int idx = find_position(mem_addr);
        if(idx == -1) {
            printf("Word not in memory.\n");
            exit(0);
        }
        Node *curr = head;
        for(int i = 0; i < idx; i++) {
            curr = curr->next;
        }
        RF[rt] = curr->data;
    }
    else if(strcmp(instr.funct_name, "lui") == 0) {
        RF[rt] = imm << 16;
    }
    curr_pc += 4;
}


void execute_j(int i) {
    InstrDict instr = instr_list[i];
    int a0 = RF[4];
    int a1 = RF[5];
    int v0 = RF[2];

    if(strcmp(instr.instr, "j gcd") == 0) {
        RF[2] = gcd(a0, a1);
        curr_pc = curr_pc + 4;
    }
    else if(strcmp(instr.instr, "jal gcd") == 0) {
        RF[2] = gcd(a0, a1);
        RF[31] = curr_pc + 4;
        curr_pc = curr_pc + 4;
    }
    else if(strcmp(instr.funct_name, "j") == 0) {
        curr_pc = instr.jta * 4; 
    }
    else if(strcmp(instr.funct_name, "jal") == 0) {
        RF[31] = curr_pc + 4;
        curr_pc = instr.jta * 4; 
    }
}


void execute_branch(int i) {
    InstrDict instr = instr_list[i];
    int rs = instr.rs_num;
    int rt = instr.rt_num;
    int imm = instr.imm;

    if(strcmp(instr.funct_name, "beq") == 0) {
        if(RF[rs] == RF[rt]) {
            curr_pc = curr_pc + 4 + (imm * 4); 
        }
        else curr_pc += 4;
    } 
    else if(strcmp(instr.funct_name, "bne") == 0) {
        if(RF[rs] != RF[rt]) {
            curr_pc = curr_pc + 4 + (imm * 4);
        }
        else curr_pc += 4;
    }  
}


void execute_pseudo(int i) {
    InstrDict instr = instr_list[i];
    int rs = instr_list[i].rs_num;
    int rt = instr_list[i].rt_num;
    int rd = instr_list[i].rd_num;
    int imm = instr_list[i].imm;
    
    if(strcmp(instr.funct_name, "li") == 0) {
        curr_pc += 4;
        RF[rt] = imm;
    }
    else if(strcmp(instr.funct_name, "move") == 0) {
        curr_pc += 4;
        RF[rd] = RF[rs];
    }
    else if(strcmp(instr.funct_name, "lw") == 0) {
        curr_pc += 8;
        int mem_addr = instr.data_loc; 
        int idx = find_position(mem_addr);
        if(idx == -1) {
            printf("Word not in memory.\n");
            exit(0);
        }

        Node *curr = head;
        for(int i = 0; i < idx; i++) {
            curr = curr->next;
        }
        RF[rt] = curr->data;
    }
    else if(strcmp(instr.funct_name, "la") == 0) {
        curr_pc += 8;
        RF[rt] = instr.data_loc;
    }
}


void execute_macro(int i) {
    InstrDict instr = instr_list[i];

    if(strcmp(instr.funct_name, "print_str") == 0) {
        if(strcmp(instr_list[i].asciiz_value, "\\n\n") == 0) {
            printf("\n");
        }
        else printf("%s", instr_list[i].asciiz_value);
        curr_pc += 16;
    }
    else if(strcmp(instr.funct_name, "read_str") == 0) {
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
    else if(strcmp(instr.funct_name, "print_integer") == 0) {
        printf("%d", instr.macro_inp_1);
        curr_pc += 12;
    }
    else if(strcmp(instr.funct_name, "read_integer") == 0) {
        RF[24] = instr.macro_inp_1;
        curr_pc += 12;
    }

}


int gcd(int a, int b) {
    if(a == 0) return b;
    return gcd(b % a, a);
}


void convert_r_to_machine(FILE *fp, char *funct_name, char *rd, char *rs, char *rt, int i) {
    char to_write[MACHINE_LEN] = "";
    int k;
    // op code
    strcat(to_write, "000000");
    // rs
    k = find_index_in_lut(i, rs);
    strcat(to_write, LUT[k].bin_code);
    instr_list[i].rs_num = LUT[k].reg_num;
    if(strcmp(instr_list[i].funct_name, "jr") != 0) {
        // rt
        k = find_index_in_lut(i, rt);
        strcat(to_write, LUT[k].bin_code);
        instr_list[i].rt_num = LUT[k].reg_num;
        // rd
        k = find_index_in_lut(i, rd);
        strcat(to_write, LUT[k].bin_code);
        instr_list[i].rd_num = LUT[k].reg_num;
    } else strcat(to_write, "0000000000");
    // shamt
    strcat(to_write, "00000");
    // funct
    k = find_index_in_lut(i, funct_name);
    strcat(to_write, LUT[k].bin_code);
    strcat(to_write, "\n\0");

    fprintf(fp, to_write);
}


void convert_i_to_machine(FILE *fp, char *funct_name, char *rs, char *rt, int imm, int i) {
    char to_write[MACHINE_LEN] = "";
    int k;
    // op code
    k = find_index_in_lut(i, funct_name);
    strcat(to_write, LUT[k].bin_code);
    // rs
    if(strcmp(funct_name, "lui") == 0) {
        strcat(to_write, "00000");
    }
    else {
        k = find_index_in_lut(i, rs);
        strcat(to_write, LUT[k].bin_code);
        instr_list[i].rs_num = LUT[k].reg_num;
    }
    // rt
    k = find_index_in_lut(i, rt);
    strcat(to_write, LUT[k].bin_code);
    instr_list[i].rt_num = LUT[k].reg_num;
    // imm
    char *bin = int_to_binary(imm, 16);
    strcat(to_write, bin);
    strcat(to_write, "\n\0");

    fprintf(fp, to_write);
}


void convert_j_to_machine(FILE *fp, char *funct_name, int imm, int i) {
    char to_write[MACHINE_LEN] = "";
    int k;
    // op code
    k = find_index_in_lut(i, funct_name);
    strcat(to_write, LUT[k].bin_code);
    // jta
    char *bin = int_to_binary(imm, 26);
    strcat(to_write, bin);
    strcat(to_write, "\n\0");

    fprintf(fp, to_write);
}


void convert_ps_to_machine(FILE *fp, int i){
    InstrDict instr = instr_list[i];
    if(strcmp(instr.funct_name, "move") == 0){
        convert_r_to_machine(fp, "add", instr.rd, instr.rs, "$0", i);
    }
    else if(strcmp(instr.funct_name, "li") == 0){
        convert_i_to_machine(fp, "ori", "$0", instr.rt, instr.imm, i);
    }
    else if(strcmp(instr.funct_name, "la") == 0){
        fprintf(fp, "00111100000000010001000000000000\n\0");
        convert_i_to_machine(fp, "ori", "$at", instr.rt, instr.data_loc & 65535, i);
    }
    else if(strcmp(instr.funct_name, "lw") == 0){
        fprintf(fp, "00111100000000010001000000000000\n\0");
        convert_i_to_machine(fp, "lw", "$at", instr.rt, instr.data_loc - 0x10000000, i);
    }
}


void convert_macro_to_machine(FILE *fp, int i) {
    InstrDict instr = instr_list[i];
    char *syscall = "00000000000000000000000000000000\n\0";
    char *luiData = "00111100000000010001000000000000\n\0";

    // read_str(label,len)
    if(strcmp(instr_list[i].funct_name, "read_str") == 0) {
        fprintf(fp, luiData);
        convert_i_to_machine(fp, "ori", "$at", "$a0", instr.data_loc - 0x10000000, i); //label
        convert_i_to_machine(fp, "ori", "$0", "$a1", instr.macro_inp_2, i); //len
        convert_i_to_machine(fp, "ori", "$0", "$v0", 8, i);
        fprintf(fp, syscall);
    }
    // print_str(label)
    else if(strcmp(instr_list[i].funct_name, "print_str") == 0) {
        fprintf(fp, luiData);
        convert_i_to_machine(fp, "ori", "$at", "$a0", instr.data_loc - 0x10000000, i); //label
        convert_i_to_machine(fp, "ori", "$0", "$v0", 4, i);
        fprintf(fp, syscall);
    }
    // read_integer(rd)
    else if(strcmp(instr_list[i].funct_name, "read_integer") == 0) {
        convert_i_to_machine(fp, "ori", "$0", "$v0", 5, i);
        fprintf(fp, syscall);
        convert_r_to_machine(fp, "add", "$t8", "$v0", "$0", i);
    }
    // print_integer(rs)
    else if(strcmp(instr_list[i].funct_name, "print_integer") == 0) {
        convert_i_to_machine(fp, "ori", "$0", "$a0", instr_list[i].macro_inp_1, i);
        convert_i_to_machine(fp, "ori", "$0", "$v0", 1, i);
        fprintf(fp, syscall);
    }
    // exit
    else if(strcmp(instr_list[i].funct_name, "exit") == 0) {
        convert_i_to_machine(fp, "ori", "$0", "$v0", 10, i);
        fprintf(fp, syscall);
    }
}


void compute_jta(int i, char *token) {
    char *binary;
    char *truncated_bin;
    for(int j = 0; j < n; j++) {
        if(instr_list[j].text_symbol != NULL && strcmp(instr_list[j].text_symbol, token) == 0) {
            instr_list[i].jta = (instr_list[j].text_loc << 4) >> 6;                        
            break;
        }
    }        
}


void get_first_two_op(int i, char *token, bool isBranch) {
    char *op_token;  
    if(isBranch) {
        op_token = strtok(token, ",");
        instr_list[i].rs = strdup(op_token); 
        op_token = strtok(NULL, ",");
        instr_list[i].rt = strdup(op_token);
        op_token = strtok(NULL, ",");
        for(int j = 0; j < n; j++) {
            if(instr_list[j].text_symbol != NULL && strcmp(instr_list[j].text_symbol, op_token) == 0) {
                instr_list[i].imm = (((int) instr_list[j].text_loc - (int) instr_list[i].text_loc) / 4) - 1;
                break;
            }
        }
    } else {
        op_token = strtok(token, ",");
        instr_list[i].rt = strdup(op_token); 
        op_token = strtok(NULL, ",");
        instr_list[i].rs = strdup(op_token);
        op_token = strtok(NULL, ",");
        instr_list[i].imm = atoi(op_token); 
        op_token = strtok(NULL, ",");
    }
}


char *int_to_binary(int num, int bits) {
    char *bin = (char *) malloc(bits + 1);

    int k;
    if(num < 0) {
        num = (1 << bits) + num;
    }
    for(k = bits - 1; k >= 0; k--) {
        int bit = (num >> k) & 1;
        bin[bits - 1 - k] = bit + '0';
    }
    bin[bits] = '\0';
    return bin;
}


void assign_val_to_symbol(int i, int word_val, char *ascii_val, char *datalabel) {
    instr_list[i].word_value = word_val;
    instr_list[i].asciiz_value = strdup(ascii_val);
    instr_list[i].data_symbol = strdup(datalabel);
    for(int j = 0; j <= i; j++) {
        if(instr_list[j].data_symbol != NULL && strcmp(instr_list[j].data_symbol, datalabel) == 0) {
            instr_list[i].data_loc = instr_list[j].data_loc;
            if(word_val != 0) {
                instr_list[j].word_value = word_val;
            }
            else if(ascii_val != NULL) {
                instr_list[j].asciiz_value = strdup(ascii_val);
            }
        }
    }
}


bool check_duplicate_symbol(int i, char *token) {
    int flag = 0;
    for(int j = 0; j < i; j++) {
        if(instr_list[j].data_symbol != NULL && strcmp(instr_list[j].data_symbol, token) == 0) {
            instr_list[i].data_loc = instr_list[j].data_loc;
            // instr_list[i].macro_inp_1 = instr_list[j].macro_inp_1;
            // instr_list[i].macro_inp_2 = instr_list[j].macro_inp_2;
            instr_list[i].word_value = instr_list[j].word_value;
            if(instr_list[i].asciiz_value != NULL) {
                instr_list[i].asciiz_value = strdup(instr_list[j].asciiz_value);
            }
            flag = 1;
        }
    }

    if(flag == 1) return true;
    else return false;
}


void write_to_symbol_table(FILE *fp, int i) {
    InstrDict instr = instr_list[i];
    char to_write[64] = "";
    char loc_str[11];
    // what to_write is instruction + <tab> + memory locatiom

    if(instr_list[i].data_symbol != NULL){
        bool is_dupe_symbol = check_duplicate_symbol(i, instr_list[i].data_symbol);
        if(is_dupe_symbol) return;
    }

    // instruction
    if(instr_list[i].text_symbol != NULL) {
        strcat(to_write, instr_list[i].text_symbol);
        strcat(to_write, "    "); 
        sprintf(loc_str, "0x%08x", instr_list[i].text_loc);
        strcat(to_write, loc_str);
        strcat(to_write, "\n");
        fprintf(fp, to_write);
    }

    to_write[0] = '\0';
    loc_str[0] = '\0';

    if(instr_list[i].data_symbol != NULL) {
        strcat(to_write, instr_list[i].data_symbol);
        strcat(to_write, "    "); 
        sprintf(loc_str, "0x%08x", instr_list[i].data_loc);
        strcat(to_write, loc_str);
        strcat(to_write, "\n");
        fprintf(fp, to_write);
    }
}


// prints the data of a node
void print_node(int idx) {
    Node *curr = head;
    for(int i = 0; i < idx; i++) {
        curr = curr->next; 
    }
    printf("%s", curr->string);

}


// adds a node at the end of the linked list
void add_node(int mem_addr, char *datalabel, int data, char *string) {
    Node *new = (Node *) malloc(sizeof(Node));
    new->datalabel = strdup(datalabel);
    new->mem_addr = mem_addr;
    new->data = data;
    new->string = strdup(string);
    new->next = NULL;

    if(head == NULL) {
        head = new;
        return;
    }

    Node *curr = head;
    while(curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = new;
}


// returns the index (0-indexed) of a node given a memory address
// if no nodes exists, returns -1
int find_position(int mem_addr) {
    Node *temp = head;
    int i = 0;
    while(temp != NULL) {
        if(temp->mem_addr == mem_addr) return i;
        i++;
        temp = temp->next;
    }
    return -1;
}