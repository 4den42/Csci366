//
// Created by carson on 11/15/21.
//

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "assembler.h"

char *ASM_ERROR_UNKNOWN_INSTRUCTION = "Unknown Assembly Instruction";
char *ASM_ERROR_ARG_REQUIRED = "Argument Required";
char *ASM_ERROR_BAD_LABEL = "Bad Label";
char *ASM_ERROR_OUT_OF_RANGE = "Number is out of range";

//=========================================================
//  All the instructions available on the LMSM architecture
//=========================================================
const char *INSTRUCTIONS[28] =
        {"ADD", "SUB", "LDA", "STA", "BRA", "BRZ", "BRP", "INP", "OUT", "HLT", "COB", "DAT",
         "LDI",
         "JAL", "CALL", "RET",
         "SPUSH", "SPUSHI", "SPOP", "SDUP", "SDROP", "SSWAP", "SADD", "SSUB", "SMAX", "SMIN", "SMUL", "SDIV"
        };
const int INSTRUCTION_COUNT = 28;

//===================================================================
//  All the instructions that require an arg on the LMSM architecture
//===================================================================
const char *ARG_INSTRUCTIONS[11] =
        {"ADD", "SUB", "LDA", "STA", "BRA", "BRZ", "BRP", "DAT",
         "LDI",
         "CALL",
         "SPUSHI"
        };
const int ARG_INSTRUCTION_COUNT = 11;

//======================================================
// Constructors/Destructors
//======================================================

asm_instruction * asm_make_instruction(char* type, char *label, char *label_reference, int value, asm_instruction * predecessor) {
    asm_instruction *new_instruction = calloc(1, sizeof(asm_instruction));
    new_instruction->instruction = type;
    new_instruction->label = label;
    new_instruction->label_reference = label_reference;
    new_instruction->value = value;
    new_instruction->next = NULL;
    if (predecessor != NULL) {
        predecessor->next = new_instruction;
        new_instruction->offset = predecessor->offset + predecessor->slots;
    } else {
        new_instruction->offset = 0;
    }
    if(predecessor != NULL) {        //Handle case of null pointer
        new_instruction->slots = predecessor->slots;
    }else if(asm_is_instruction(type)){
        if((strcmp(type,"SPUSHI") == 0) || (strcmp(type,"ADD") == 0) || (strcmp(type,"SUB") == 0) || (strcmp(type,"LDI") == 0)){
            new_instruction->slots = 2;
        }else if(strcmp(type,"CALL") == 0){
            new_instruction->slots = 3;
        }
        //new_instruction->slots = value;  //Set number of slots to 1 if pointer was null
    }else{
        new_instruction->slots = 1;
    }
    return new_instruction;
}

void asm_delete_instruction(asm_instruction *instruction) {
    if (instruction == NULL) {
        return;
    }
    asm_delete_instruction(instruction->next);
    free(instruction);
}

asm_compilation_result * asm_make_compilation_result() {
    asm_compilation_result *result = calloc(1, sizeof(asm_compilation_result));
    return result;
}

void asm_delete_compilation_result(asm_compilation_result *result) {
    asm_delete_instruction(result->root);
    free(result);
}

//======================================================
// Helpers
//======================================================
int asm_is_instruction(char * token) {
    for (int i = 0; i < INSTRUCTION_COUNT; ++i) {
        if (strcmp(token, INSTRUCTIONS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int asm_instruction_requires_arg(char * token) {
    for (int i = 0; i < ARG_INSTRUCTION_COUNT; ++i) {
        if (strcmp(token, ARG_INSTRUCTIONS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int asm_is_num(char * token){
    if (*token == '-') { // allow a leading negative
        token++;
        if (*token == '\0') {
            return 0;
        }
    }
    while (*token != '\0') {
        if (*token < '0' || '9' < *token) {
            return 0;
        }
        token++;
    }
    return 1;
}

int asm_find_label(asm_instruction *root, char *label) {
    asm_instruction* temp = root;
    while (temp != NULL) {
        if ((label != NULL) && (temp->label != NULL) && (strcmp(label, temp->label) == 0)) {
           // printf("Label value is: %d", temp->value);
            return temp->offset;           //If label is found return its value
        }

        temp = temp->next;
    }
    return -1;                      // Label not found
}


//======================================================
// Assembly Parsing/Scanning
//======================================================

void asm_parse_src(asm_compilation_result * result, char * original_src){

    char * src = calloc(strlen(original_src) + 1, sizeof(char));  // copy over so strtok can mutate
    strcat(src, original_src);
    asm_instruction * last_instruction = NULL;
    asm_instruction * current_instruction = NULL;
    int rangeError = 0;
    //
    //       generate the following errors as appropriate:
    //
    //       ASM_ERROR_UNKNOWN_INSTRUCTION - when an unknown instruction is encountered
    //       ASM_ERrOR_ARG_REQUIRED        - when an instruction does not have a proper argument passed to it
    //       ASM_ERROR_OUT_OF_RANGE        - when a number argument is out of range (-999 to 999)
    //
    //       store the error in result->error

    char *instruction = strtok(src, " \n");  //instruction is the label i.e. "ADD"
    while (instruction != NULL){                     //Read through all
            if(asm_is_instruction(instruction)) {
                current_instruction = asm_make_instruction(strdup(instruction),NULL,NULL,0,NULL);
            }else if(asm_is_num(instruction) == 1){
                int val = atoi(instruction);
                if((val < 999) && (val > -999)) {
                    if (last_instruction != NULL) {
                        current_instruction->value = atoi(instruction);
                        last_instruction->value = atoi(instruction);  //If current token is a number, relate it to its previous instruction
                        break;
                    }else{
                        result->error = strdup(ASM_ERROR_UNKNOWN_INSTRUCTION);      //Ensure number is in range
                        break;
                    }
                }else{
                    result->error = ASM_ERROR_OUT_OF_RANGE;//^
                    rangeError += 1;                                //quick fix
                    break;
                }
            }else{
                current_instruction = malloc(sizeof(asm_instruction));
                current_instruction->label = strdup(instruction);          //Else create label
                current_instruction->instruction = NULL;
                current_instruction->next = NULL;
                current_instruction->value = 0;

            }
            if (last_instruction == NULL) {
               result->root = current_instruction;                  // If first instruction, make it the root
               last_instruction = current_instruction;              //Update the last_instruction pointer
            }else if((last_instruction->instruction == NULL) && (last_instruction->label != NULL) && (asm_is_instruction(instruction) == 1)) {
                last_instruction->instruction = strdup(instruction);      //If the last instruction is just a label, add its instruction
            }else if((last_instruction->label == NULL) && (current_instruction->label != NULL)) {
                last_instruction->label_reference = strdup(instruction);           //If the last instruction doesn't have a label, make the non instruction/value its label
            }else if((last_instruction->label != NULL) && (asm_is_instruction(instruction) == 0)){ //double label check
                result->error = ASM_ERROR_UNKNOWN_INSTRUCTION;
                break;
            } else {
                current_instruction->offset = last_instruction->offset + 1;
                last_instruction->next = current_instruction;           // Link the last instruction to the current one
                last_instruction = current_instruction;              //Update the last_instruction pointer

            }
            instruction = strtok(NULL, " \n");
    }
    if((current_instruction->label == NULL) && (current_instruction->value == 0) && ((asm_instruction_requires_arg(last_instruction->instruction)) == 1) && (rangeError == 0)){
                result->error = ASM_ERROR_ARG_REQUIRED;
    }

}

//======================================================
// Machine Code Generation
//======================================================

void asm_gen_code_for_instruction(asm_compilation_result  * result, asm_instruction *instruction) {
    // note that some instructions will take up multiple slots
    //
    // note that if the instruction has a label reference rather than a raw number reference
    // you will need to look it up with `asm_find_label` and, if the label does not exist,
    // report the error as ASM_ERROR_BAD_LABEL
    int value_for_instruction = instruction->value;
    if(instruction->label_reference != NULL) {
        int labelOffset = asm_find_label(result->root, instruction->label_reference);
        if(labelOffset != -1) {
            value_for_instruction += labelOffset;   //Check and handle labels/label references if they exist
        } else {
            result->error = ASM_ERROR_BAD_LABEL;   //Throw error for nonexistent label being referenced
            return;
        }
    }
    printf("labelValue: %d",value_for_instruction);
    if(strcmp("HALT",instruction->instruction) == 0){
        result->code[instruction->offset] = 0 + value_for_instruction;  //check
    }else if (strcmp("ADD", instruction->instruction) == 0) {
        result->code[instruction->offset] = 100 + value_for_instruction;
    } else if(strcmp("SUB",instruction->instruction) == 0){
        result->code[instruction->offset] = 200 + value_for_instruction;
    } else if(strcmp("STA",instruction->instruction) == 0){
        result->code[instruction->offset] = 300 + value_for_instruction;
    }else if(strcmp("LDI",instruction->instruction) == 0){
        result->code[instruction->offset] = 400 + value_for_instruction;
    }else if(strcmp("LDA",instruction->instruction) == 0){
        result->code[instruction->offset] = 500 + value_for_instruction;
    }else if(strcmp("BRA",instruction->instruction) == 0){
        result->code[instruction->offset] = 600 + value_for_instruction;
    }else if(strcmp("BRZ",instruction->instruction) == 0){
        result->code[instruction->offset] = 700 + value_for_instruction;
    }else if(strcmp("BRP",instruction->instruction) == 0){
        result->code[instruction->offset] = 800 + value_for_instruction;
    }else if(strcmp("INP",instruction->instruction) == 0){
        result->code[instruction->offset] = 901;
    }else if(strcmp("OUT",instruction->instruction) == 0){
        result->code[instruction->offset] = 902;
    }else if(strcmp("DAT",instruction->instruction) == 0) {
        result->code[instruction->offset] = 0 + value_for_instruction;
    }else if(strcmp("RET",instruction->instruction) == 0){
        result->code[instruction->offset] = 911;
    }else if(strcmp("SPUSH",instruction->instruction) == 0){
        result->code[instruction->offset] = 920;
    }else if(strcmp("SPUSHI",instruction->instruction) == 0){
        result->code[instruction->offset] = 400 + value_for_instruction;
        result->code[instruction->offset + 1] = 920;
    }else if(strcmp("SPOP",instruction->instruction) == 0){
        result->code[instruction->offset] = 921;
    }else if(strcmp("SDUP",instruction->instruction) == 0){
        result->code[instruction->offset] = 922;
    }else if(strcmp("SDROP",instruction->instruction) == 0){
        result->code[instruction->offset] = 923;
    }else if(strcmp("SSWAP",instruction->instruction) == 0){
        result->code[instruction->offset] = 924;
    }else if(strcmp("SADD",instruction->instruction) == 0){  //If else statements tp set code values
        result->code[instruction->offset] = 930;
    }else if(strcmp("SSUB",instruction->instruction) == 0){
        result->code[instruction->offset] = 931;
    }else if(strcmp("SMUL",instruction->instruction) == 0){
        result->code[instruction->offset] = 932;
    }else if(strcmp("SDIV",instruction->instruction) == 0){
        result->code[instruction->offset] = 933;
    }else if(strcmp("SMAX",instruction->instruction) == 0){
        result->code[instruction->offset] = 934;
    }else if(strcmp("SMIN",instruction->instruction) == 0){
        result->code[instruction->offset] = 935;
    }else if(strcmp("CALL",instruction->instruction) == 0){
        result->code[instruction->offset] = 400 + value_for_instruction;
        result->code[instruction->offset + 1] = 920;
        result->code[instruction->offset + 2] = 910;
    }else{
        result->code[instruction->offset] = 0;

    }
}

void asm_gen_code(asm_compilation_result * result) {
    asm_instruction * current = result->root;
    while (current != NULL) {
        asm_gen_code_for_instruction(result, current);
        current = current->next;
    }
}

//======================================================
// Main API
//======================================================

asm_compilation_result * asm_assemble(char *src) {
    asm_compilation_result * result = asm_make_compilation_result();
    asm_parse_src(result, src);
    asm_gen_code(result);
    return result;
}
