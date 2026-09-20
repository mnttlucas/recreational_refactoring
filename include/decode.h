#ifndef	DECODE_H
#define	DECODE_H

#include <stdint.h>
#include <stdio.h>

#include "instruction.h"

#define FIELD(start, end, value) \
	(instruction_field) {value, start, end}

typedef struct
{
	int32_t	value;
	uint8_t	start_bit;
	uint8_t	end_bit;
} instruction_field;

int32_t handle_sign(char *param, instruction *instr);
void build_instruction_bin(instruction_field *arr, size_t field_count, uint8_t *bin_arr, char *hex_arr);
void finalize_signed_value(int32_t *value, instruction *instr, uint8_t start_bit, uint8_t end_bit);

void build_branch_r1_instruction(instruction *instr, int op_code);
void build_branch_r2_instruction(instruction *instr, int op_code);
void build_memory_instruction(instruction *instr, int op_code);
void build_r2_instruction(instruction *instr, int funct_code);
void build_r3_immediate_instruction(instruction *instr, int op_code);
void build_r3_instruction(instruction *instr, int funct);
void build_rd_instruction(instruction *instr, int funct_code);
void build_rd_rs_instruction(instruction *instr, int funct_code);
void build_rs_instruction(instruction *instr, int funct_code);
void build_rt_immediate_instruction(instruction *instr, int op_code);
void build_shift_instruction(instruction *instr, int funct_code);
void build_target_instruction(instruction *instr, int op_code);
void build_variable_shift_instruction(instruction *instr, int funct_code);

void decode_branch_r1_operands(char *line, instruction *instr);
void decode_branch_r2_operands(char *line, instruction *instr);
void decode_memory_operands(char *line, instruction *instr);
void decode_r2_operands(char *line, instruction *instr);
void decode_r3_immediate_operands(char *line, instruction *instr);
void decode_r3_operands(char *line, instruction *instr);
void decode_rd_operand(char *line, instruction *instr);
void decode_rd_rs_operands(char *line, instruction *instr);
void decode_rs_operand(char *line, instruction *instr);
void decode_rt_immediate_operands(char *line, instruction *instr);
void decode_shift_operands(char *line, instruction *instr);
void decode_target_operands(char *line, instruction *instr);
void decode_variable_shift_operands(char *line, instruction *instr);

instruction	decode_instruction(int mode, FILE *fichier);

#endif	/* DECODE_H */