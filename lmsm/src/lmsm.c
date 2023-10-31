#include "lmsm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>


//======================================================
//  Utilities
//======================================================

void lmsm_cap_value(int * val){
    *val = fmin(fmax(*val, -999), 999);
   //TODO - implement capping the value pointed to by this pointer between 999 and -999
}

int lmsm_has_two_values_on_stack(lmsm *our_little_machine) {
    //TODO - return 0 if there are not two values on the stack
    int SP = our_little_machine->stack_pointer;
    if(SP < 199){
        return 1;
    }
    return 0;
}

//======================================================
//  Instruction Implementation
//======================================================

void lmsm_i_jal(lmsm *our_little_machine) {
}

void lmsm_i_ret(lmsm *our_little_machine) {
}

void lmsm_i_push(lmsm *our_little_machine) {
    // we want to push the value of the accumulator onto the value stack (starting at pos 199)
    our_little_machine->stack_pointer--;
    our_little_machine->memory[our_little_machine->stack_pointer] = our_little_machine->accumulator;
}

void lmsm_i_pop(lmsm *our_little_machine) {
}

void lmsm_i_dup(lmsm *our_little_machine) {
    our_little_machine->stack_pointer--;
    our_little_machine->memory[our_little_machine->stack_pointer] =
            our_little_machine->memory[our_little_machine->stack_pointer + 1];
}

void lmsm_i_drop(lmsm *our_little_machine) {
}

void lmsm_i_swap(lmsm *our_little_machine) {
}

void lmsm_i_sadd(lmsm *our_little_machine) {
    int value1 = our_little_machine->memory[our_little_machine->stack_pointer];
    int value2 = our_little_machine->memory[our_little_machine->stack_pointer + 1];
    int result = value1 + value2;
    our_little_machine->stack_pointer++;
    our_little_machine->memory[our_little_machine->stack_pointer] = result;
}

void lmsm_i_ssub(lmsm *our_little_machine) {
}

void lmsm_i_smax(lmsm *our_little_machine) {
}

void lmsm_i_smin(lmsm *our_little_machine) {
}

void lmsm_i_smul(lmsm *our_little_machine) {
}

void lmsm_i_sdiv(lmsm *our_little_machine) {
}

void lmsm_i_out(lmsm *our_little_machine) {
    // TODO, append the current accumulator to the output_buffer in the LMSM
    int curAcc = *our_little_machine->accumulator;
    for(int i = 0; i < 400; i++){
        if(strcmp(our_little_machine->output_buffer[i],""){
            our_little_machine->output_buffer[i] = (char)curAcc;
        }
    }
}

void lmsm_i_inp(lmsm *our_little_machine) {
    // TODO read a value from the command line and store it as an int in the accumulator
    int inst = Convert.ToInt32(Console.ReadLine());
    our_little_machine->accumulator = inst;
}

void lmsm_i_load(lmsm *our_little_machine, int location) {
}

void lmsm_i_add(lmsm *our_little_machine, int location) {
    our_little_machine->accumulator += our_little_machine->memory[location];
}

void lmsm_i_sub(lmsm *our_little_machine, int location) {
    our_little_machine->accumulator -= our_little_machine->memory[location];
}

void lmsm_i_load_immediate(lmsm *our_little_machine, int value) {
}

void lmsm_i_store(lmsm *our_little_machine, int location) {
}

void lmsm_i_halt(lmsm *our_little_machine) {
}

void lmsm_i_branch_unconditional(lmsm *our_little_machine, int location) {
}

void lmsm_i_branch_if_zero(lmsm *our_little_machine, int location) {
}

void lmsm_i_branch_if_positive(lmsm *our_little_machine, int location) {
}

void lmsm_step(lmsm *our_little_machine) {
    // TODO : if the machine is not halted, we need to read the instruction in the memory slot
    //        pointed to by the program counter, bump the program counter then execute
    //        the instruction
    //Already done?
    if (our_little_machine->status != STATUS_HALTED) {
        int next_instruction = our_little_machine->memory[our_little_machine->program_counter];
        our_little_machine->program_counter++;
        our_little_machine->current_instruction = next_instruction;
        int instruction = our_little_machine->current_instruction;
        lmsm_exec_instruction(our_little_machine, instruction);
    }
}

//======================================================
//  LMSM Implementation
//======================================================

void lmsm_exec_instruction(lmsm *our_little_machine, int instruction) {

    // TODO - dispatch the rest of the instruction set and implement
    //        the instructions above
    // Also already done?

    if (instruction == 0) {
        lmsm_i_halt(our_little_machine);
    } else if (100 <= instruction && instruction <= 199) {
        lmsm_i_add(our_little_machine, instruction - 100);
    } else if (200 <= instruction && instruction <= 299) {
        lmsm_i_sub(our_little_machine, instruction - 200);
    } else if (700 <= instruction && instruction <= 799) {
        lmsm_i_branch_if_zero(our_little_machine, instruction - 700);
    } else if (instruction == 920) {
        lmsm_i_push(our_little_machine);
    } else if (instruction == 922) {
        lmsm_i_dup(our_little_machine);
    } else if (instruction == 930) {
        lmsm_i_sadd(our_little_machine);
    } else {
        our_little_machine->error_code = ERROR_UNKNOWN_INSTRUCTION;
        our_little_machine->status = STATUS_HALTED;
    }
    lmsm_cap_value(&our_little_machine->accumulator);
}

void lmsm_load(lmsm *our_little_machine, int *program, int length) {
    for (int i = 0; i < length; ++i) {
        our_little_machine->memory[i] = program[i];
    }
}

void lmsm_init(lmsm *the_machine) {
    the_machine->accumulator = 0;
    the_machine->status = STATUS_READY;
    the_machine->error_code = ERROR_NONE;
    the_machine->program_counter = 0;
    the_machine->current_instruction = 0;
    the_machine->stack_pointer = TOP_OF_MEMORY + 1;
    the_machine->return_address_pointer = TOP_OF_MEMORY - 100;
    memset(the_machine->output_buffer, 0, sizeof(char) * 1000);
    memset(the_machine->memory, 0, sizeof(int) * TOP_OF_MEMORY + 1);
}

void lmsm_reset(lmsm *our_little_machine) {
    lmsm_init(our_little_machine);
}

void lmsm_run(lmsm *our_little_machine) {
    our_little_machine->status = STATUS_RUNNING;
    while (our_little_machine->status != STATUS_HALTED) {
        lmsm_step(our_little_machine);
    }
}

lmsm *lmsm_create() {
    lmsm *the_machine = malloc(sizeof(lmsm));
    lmsm_init(the_machine);
    return the_machine;
}

void lmsm_delete(lmsm *the_machine) {
    free(the_machine);
}