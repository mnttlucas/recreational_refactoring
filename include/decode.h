#ifndef	DECODE_H
#define	DECODE_H

#include "instruction.h"

#define FIELD(start, end, value) \
	(instruction_field) {start, end, value}

typedef struct
{
	int  start_bit;
	int  end_bit;
	long value;
} instruction_field;

int handle_sign(char *param, int *negative);
void build_instruction_bin(instruction_field *arr, size_t field_count, int *bin_arr, char *hex_arr);

void build_r3_instruction(instruction *instr, int funct, int *bin_arr, char *hex_arr);

void decode_r2_operands(FILE *in, instruction *instr);
void decode_r3_operands(FILE *in, instruction *instr);
void decode_shift_operands(FILE *in, instruction *instr);
void decode_memory_operands(FILE *in, instruction *instr, int *negative);
void decode_branch_r1_operands(FILE *in, instruction *instr, int *negative);
void decode_branch_r2_operands(FILE *in, instruction *instr, int *negative);
void decode_target_operands(FILE *in, instruction *instr);
void decode_rd_operand(FILE *in, instruction *instr);

instruction	decode_instruction(int mode, FILE *fichier);

#endif	/* DECODE_H */