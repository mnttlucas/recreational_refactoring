#ifndef	DECODE_H
#define	DECODE_H

#include <stdint.h>
#include <stdio.h>

#include "instruction.h"

void build_offset_16_instruction(instruction *instr, int op_code);
void build_offset_26_instruction(instruction *instr, int op_code);
void build_rd_instruction(instruction *instr, int funct_code);
void build_rd_rs_instruction(instruction *instr, int funct_code);
void build_rd_rs_rt_instruction(instruction *instr, int funct);
void build_rd_rt_instruction(instruction *instr, int funct_code);
void build_rd_rt_rs_instruction(instruction *instr, int funct_code);
void build_rd_rt_sa_instruction(instruction *instr, int funct_code);
void build_rs_instruction(instruction *instr, int funct_code);
void build_rs_immediate_16_instruction(instruction *instr, int op_code);
void build_rs_immediate_19_instruction(instruction *instr, int op_code);
void build_rs_offset_16_instruction(instruction *instr, int op_code);
void build_rs_offset_19_instruction(instruction *instr, int op_code);
void build_rs_offset_21_instruction(instruction *instr, int op_code);
void build_rs_rt_instruction(instruction *instr, int funct_code);
void build_rs_rt_offset_16_instruction(instruction *instr, int op_code);
void build_rt_immediate_16_instruction(instruction *instr, int op_code);
void build_rt_offset_16_instruction(instruction *instr, int op_code);
void build_rt_offset_base_instruction(instruction *instr, int op_code);
void build_rt_rs_immediate_16_instruction(instruction *instr, int op_code);
void build_target_instruction(instruction *instr, int op_code);

void decode_offset_operand(char *line, instruction *instr);
void decode_rd_operand(char *line, instruction *instr);
void decode_rd_rs_operands(char *line, instruction *instr);
void decode_rd_rs_rt_operands(char *line, instruction *instr);
void decode_rd_rt_operands(char *line, instruction *instr);
void decode_rd_rt_rs_operands(char *line, instruction *instr);
void decode_rd_rt_sa_operands(char *line, instruction *instr);
void decode_rs_operand(char *line, instruction *instr);
void decode_rs_immediate_operands(char *line, instruction *instr);
void decode_rs_offset_operands(char *line, instruction *instr);
void decode_rs_rt_operands(char *line, instruction *instr);
void decode_rs_rt_offset_operands(char *line, instruction *instr);
void decode_rt_immediate_operands(char *line, instruction *instr);
void decode_rt_offset_operands(char *line, instruction *instr);
void decode_rt_offset_base_operands(char *line, instruction *instr);
void decode_rt_rs_immediate_operands(char *line, instruction *instr);
void decode_rt_rs_offset_operands(char *line, instruction *instr);
void decode_target_operands(char *line, instruction *instr);

instruction	decode_instruction(int mode, FILE *input);

#endif	/* DECODE_H */