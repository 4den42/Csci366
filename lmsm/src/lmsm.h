#ifndef LMSM_LMSM_H
#define LMSM_LMSM_H

//===================================================================
//  ENUMS for the virtual machine
//===================================================================

typedef enum machine_status {
    STATUS_RUNNING,
    STATUS_HALTED,
    STATUS_READY,
} machine_status;

typedef enum error_code {
    ERROR_NONE,
    ERROR_BAD_STACK,
    ERROR_OUTPUT_EXHAUSTED,
    ERROR_UNKNOWN_INSTRUCTION,
} error_code;

#define TOP_OF_MEMORY 199
#define OUTPUT_BUFFER_SIZE 4000

//===================================================================
//  Represents the core computational infrastructure of the
//  LMSM architecture
//===================================================================

typedef struct lmsm {
    int program_counter;
    int current_instruction;
    machine_status status;
    error_code error_code;
    int accumulator;
    int stack_pointer;
    int return_address_pointer;
    int memory[TOP_OF_MEMORY + 1];
    char output_buffer[OUTPUT_BUFFER_SIZE];
} lmsm;

//=====================================================
// API
//=====================================================

// create a new little man stack machine
lmsm * lmsm_create();

// deletes the machine
void lmsm_delete(lmsm *the_machine);

// loads a program into a little man stack machine
void lmsm_load(lmsm *our_little_machine, int program[], int length);

// run the little man machine
void lmsm_run(lmsm *our_little_machine);

// step on asm_instruction on the little man machine
void lmsm_step(lmsm *our_little_machine);

// step on asm_instruction on the little man machine
void lmsm_exec_instruction(lmsm *our_little_machine, int instruction);

void lmsm_reset(lmsm *our_little_machine);

#endif //LMSM_LMSM_H
