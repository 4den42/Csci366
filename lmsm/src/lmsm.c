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
    if(*val > 999){
        *val = 999;
    }else if(*val < -999){
        *val = -999;
    }
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
    int target_address = our_little_machine->current_instruction;

    if (our_little_machine->stack_pointer < 200) {
        // Store the return address (current program counter) on the call stack
        our_little_machine->memory[our_little_machine->stack_pointer] = our_little_machine->program_counter;
        our_little_machine->stack_pointer++;
    } else {
        // Handle call stack overflow (optional)
        // You may want to generate an error or take other action if the stack is full.
    }

    // Set the program counter to the target address for the function call
    our_little_machine->program_counter = target_address;

}

void lmsm_i_ret(lmsm *our_little_machine) {
        our_little_machine->program_counter = our_little_machine->memory[our_little_machine->return_address_pointer];
        our_little_machine->return_address_pointer--;

}

void lmsm_i_push(lmsm *our_little_machine) {
    // we want to push the value of the accumulator onto the value stack (starting at pos 199)
    our_little_machine->stack_pointer--;
    our_little_machine->memory[our_little_machine->stack_pointer] = our_little_machine->accumulator;
}

void lmsm_i_pop(lmsm *our_little_machine) {
    if(our_little_machine->stack_pointer != 200) {
        our_little_machine->accumulator = our_little_machine->memory[our_little_machine->stack_pointer];
        our_little_machine->stack_pointer++;
    }else{
        our_little_machine->status = STATUS_HALTED;
        our_little_machine->error_code = ERROR_BAD_STACK;
    }

}

void lmsm_i_dup(lmsm *our_little_machine) {
    our_little_machine->stack_pointer--;
    our_little_machine->memory[our_little_machine->stack_pointer] =
            our_little_machine->memory[our_little_machine->stack_pointer + 1];
}

void lmsm_i_drop(lmsm *our_little_machine) {
    if(our_little_machine->stack_pointer != 200) {
        our_little_machine->stack_pointer++;
    }else{
        our_little_machine->status = STATUS_HALTED;
        our_little_machine->error_code = ERROR_BAD_STACK;
    }
}

void lmsm_i_swap(lmsm *our_little_machine) {
    if(our_little_machine->stack_pointer != 200) {
        int top = our_little_machine->memory[our_little_machine->stack_pointer];
        int second = our_little_machine->memory[our_little_machine->stack_pointer + 1];
        our_little_machine->memory[our_little_machine->stack_pointer] = second;
        our_little_machine->memory[our_little_machine->stack_pointer + 1] = top;
    }else{
        our_little_machine->status = STATUS_HALTED;
        our_little_machine->error_code = ERROR_BAD_STACK;
    }
}

void lmsm_i_sadd(lmsm *our_little_machine) {
    int value1 = our_little_machine->memory[our_little_machine->stack_pointer];
    int value2 = our_little_machine->memory[our_little_machine->stack_pointer + 1];
    int result = value1 + value2;
    our_little_machine->stack_pointer++;
    lmsm_cap_value(&result);
    our_little_machine->memory[our_little_machine->stack_pointer] = result;
}

void lmsm_i_ssub(lmsm *our_little_machine) {
    int value1 = our_little_machine->memory[our_little_machine->stack_pointer];
    int value2 = our_little_machine->memory[our_little_machine->stack_pointer + 1];
    int result = value2 - value1;
    our_little_machine->stack_pointer += 1;
    lmsm_cap_value(&result);
    our_little_machine->memory[our_little_machine->stack_pointer] = result;
}

void lmsm_i_smax(lmsm *our_little_machine) {
    if (our_little_machine->stack_pointer != 200) {
        int value1 = our_little_machine->memory[our_little_machine->stack_pointer];
        int value2 = our_little_machine->memory[our_little_machine->stack_pointer + 1];
        if (value1 < value2) {
            lmsm_cap_value(&value2);
            our_little_machine->memory[our_little_machine->stack_pointer + 1] = value2;
        } else {
            lmsm_cap_value(&value1);
            our_little_machine->memory[our_little_machine->stack_pointer + 1] = value1;
        }
        our_little_machine->stack_pointer += 1;
    } else {
        our_little_machine->status = STATUS_HALTED;
        our_little_machine->error_code = ERROR_BAD_STACK;
    }
}
void lmsm_i_smin(lmsm *our_little_machine) {
    if(our_little_machine->stack_pointer != 200) {
        int value1 = our_little_machine->memory[our_little_machine->stack_pointer];
        int value2 = our_little_machine->memory[our_little_machine->stack_pointer + 1];
        if (value1 < value2) {
            lmsm_cap_value(&value1);
            our_little_machine->memory[our_little_machine->stack_pointer + 1] = value1;
        } else {
            lmsm_cap_value(&value2);
            our_little_machine->memory[our_little_machine->stack_pointer + 1] = value2;
        }
        our_little_machine->stack_pointer += 1;
    }else{
        our_little_machine->status = STATUS_HALTED;
        our_little_machine->error_code = ERROR_BAD_STACK;
    }
}


void lmsm_i_smul(lmsm *our_little_machine) {
    if(our_little_machine->stack_pointer != 200) {
        int value1 = our_little_machine->memory[our_little_machine->stack_pointer];
        int value2 = our_little_machine->memory[our_little_machine->stack_pointer + 1];
        int newv = value1 * value2;
        our_little_machine->stack_pointer ++;
        lmsm_cap_value(&newv);
        our_little_machine->memory[our_little_machine->stack_pointer] = newv;
    }else{
        our_little_machine->status = STATUS_HALTED;
        our_little_machine->error_code = ERROR_BAD_STACK;
    }
}

void lmsm_i_sdiv(lmsm *our_little_machine) {
    if(our_little_machine->stack_pointer != 200) {
        int value1 = our_little_machine->memory[our_little_machine->stack_pointer];
        int value2 = our_little_machine->memory[our_little_machine->stack_pointer + 1];
        int newv = value2 / value1;
        our_little_machine->stack_pointer ++;
        lmsm_cap_value(&newv);
        our_little_machine->memory[our_little_machine->stack_pointer] = newv;
        our_little_machine->program_counter = newv;
    }else{
        our_little_machine->status = STATUS_HALTED;
        our_little_machine->error_code = ERROR_BAD_STACK;
    }

}

void lmsm_i_out(lmsm *our_little_machine) {
    // TODO, append the current accumulator to the output_buffer in the LMSM
    // Convert the accumulator value to a string
    int curAcc = our_little_machine->accumulator;
    char numc[12];  // Assuming a maximum of 11 digits plus null-terminator
    snprintf(numc, sizeof(numc), "%d", curAcc);

    // Append a space and the string to the output_buffer
    strcat(our_little_machine->output_buffer, numc); //w3schools inspired
    strcat(our_little_machine->output_buffer, " ");
}

void lmsm_i_inp(lmsm *our_little_machine) {
    // TODO read a value from the command line and store it as an int in the accumulator
    int inst;

    // Prompt the user for input
    printf("Enter an integer: ");

    // Read an integer from the command line
    if (scanf("%d", &inst) != 1) {
        // Input error handling, if needed
        printf("Invalid input.\n");
    } else {
        our_little_machine->accumulator = inst;
    }

}
    void lmsm_i_load(lmsm *our_little_machine, int location) {
    our_little_machine->accumulator = our_little_machine->memory[location];
}

void lmsm_i_add(lmsm *our_little_machine, int location) {
    our_little_machine->accumulator += our_little_machine->memory[location];
}

void lmsm_i_sub(lmsm *our_little_machine, int location) {
    our_little_machine->accumulator -= our_little_machine->memory[location];
}

void lmsm_i_load_immediate(lmsm *our_little_machine, int value) {
    our_little_machine->accumulator = value;
}

void lmsm_i_store(lmsm *our_little_machine, int location) {
    our_little_machine->memory[location] = our_little_machine->accumulator;
}

void lmsm_i_halt(lmsm *our_little_machine) {
    our_little_machine->status = STATUS_HALTED;
}

void lmsm_i_branch_unconditional(lmsm *our_little_machine, int location) {
    our_little_machine->program_counter = location;
    our_little_machine->current_instruction = our_little_machine->memory[location];
}

void lmsm_i_branch_if_zero(lmsm *our_little_machine, int location) {
    if(our_little_machine->accumulator == 0){
        lmsm_i_branch_unconditional(our_little_machine, location);
    }
}

void lmsm_i_branch_if_positive(lmsm *our_little_machine, int location) {
    if(our_little_machine->accumulator >= 0){
        lmsm_i_branch_unconditional(our_little_machine, location);
    }
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
    } else if (300 <= instruction && instruction <= 399) {
        lmsm_i_store(our_little_machine, instruction - 300);
    } else if (400 <= instruction && instruction <= 499) {
        lmsm_i_load_immediate(our_little_machine, instruction - 400);
    } else if (500 <= instruction && instruction <= 599) {
        lmsm_i_load(our_little_machine, instruction - 500);
    } else if (600 <= instruction && instruction <= 699) {
        lmsm_i_branch_unconditional(our_little_machine, instruction - 600);
    } else if (700 <= instruction && instruction <= 799) {
        lmsm_i_branch_if_zero(our_little_machine, instruction - 700);
    } else if (800 <= instruction && instruction <= 899) {
        lmsm_i_branch_if_positive(our_little_machine, instruction - 800);
    } else if (instruction == 902) {
        lmsm_i_out(our_little_machine);
    } else if (instruction == 910) {
        lmsm_i_jal(our_little_machine);
    } else if (instruction == 912) {
        lmsm_i_ret(our_little_machine);
    } else if (instruction == 920) {
        lmsm_i_push(our_little_machine);
    } else if (instruction == 921) {
        lmsm_i_pop(our_little_machine);
    } else if (instruction == 922) {
        lmsm_i_dup(our_little_machine);
    } else if (instruction == 923) {
        lmsm_i_drop(our_little_machine);
    } else if (instruction == 924) {
        lmsm_i_swap(our_little_machine);
    } else if (instruction == 930) {
        lmsm_i_sadd(our_little_machine);
    } else if (instruction == 931) {
        lmsm_i_ssub(our_little_machine);
    } else if (instruction == 932) {
        lmsm_i_smul(our_little_machine);
    } else if (instruction == 933) {
        lmsm_i_sdiv(our_little_machine);
    } else if (instruction == 934) {
        lmsm_i_smax(our_little_machine);
    } else if (instruction == 935) {
        lmsm_i_smin(our_little_machine);
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