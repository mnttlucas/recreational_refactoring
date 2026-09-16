#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decode.h"
#include "instruction.h"
#include "utils.h"

static const instruction_desc *find_instruction(const char *mnemonic)
{
	for(size_t i = 0; i < ARR_SIZE(instruction_table); i++)
		if(!strcmp(mnemonic, instruction_table[i].mnemonic)) return(&instruction_table[i]);
	return(NULL);
}

void build_instruction_bin(instruction_field *arr, size_t field_count, int *bin_arr, char *hex_arr)
{
	for(size_t i = 0; i < field_count; i++) long_to_bin_arr(arr[i].start_bit, arr[i].end_bit, arr[i].value, bin_arr);
	bin_arr_to_hex_arr(bin_arr, hex_arr);
}

int handle_sign(char *param, instruction *instr)
{
	if(is_negative(param))
	{
		instr->negative = 1;
		remove_sign(param);
	}

	return(atoi(param));
}

void finalize_signed_value(int *value, instruction *instr)
{
	if(instr->negative)
	{
		*value *= -1;
		bin_twos_complement(IMM_START, IMM_END, instr->instr_bin);
		bin_arr_to_hex_arr(instr->instr_bin, instr->instr_hex);
	}
}

void build_branch_r1_instruction(instruction *instr, int op_code)
{
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, op_code),
		FIELD(RS_START, RS_END, instr->rs),
		FIELD(IMM_START, IMM_END, instr->offset)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr->instr_bin, instr->instr_hex);
	finalize_signed_value(&instr->offset, instr);
}

void build_branch_r2_instruction(instruction *instr, int op_code)
{
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, op_code),
		FIELD(RS_START, RS_END, instr->rs),
		FIELD(RT_START, RT_END, instr->rt),
		FIELD(IMM_START, IMM_END, instr->offset)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr->instr_bin, instr->instr_hex);
	finalize_signed_value(&instr->offset, instr);
}

void build_memory_instruction(instruction *instr, int op_code)
{
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, op_code),
		FIELD(RS_START, RS_END, instr->base),
		FIELD(RT_START, RT_END, instr->rt),
		FIELD(IMM_START, IMM_END, instr->offset)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr->instr_bin, instr->instr_hex);
	finalize_signed_value(&instr->offset, instr);
}

void build_r2_instruction(instruction *instr, int funct_code)
{
	instruction_field fields[] = {
		FIELD(RS_START, RS_END, instr->rs),
		FIELD(RT_START, RT_END, instr->rt),
		FIELD(FUNCT_START, FUNCT_END, funct_code)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr->instr_bin, instr->instr_hex);
}

void build_r3_immediate_instruction(instruction *instr, int op_code)
{
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, op_code),
		FIELD(RS_START, RS_END, instr->rs),
		FIELD(RT_START, RT_END, instr->rt),
		FIELD(IMM_START, IMM_END, instr->immediate)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr->instr_bin, instr->instr_hex);
	finalize_signed_value(&instr->immediate, instr);
}

void build_r3_instruction(instruction *instr, int funct_code)
{
	instruction_field fields[] = {
		FIELD(RS_START, RS_END, instr->rs),
		FIELD(RT_START, RT_END, instr->rt),
		FIELD(RD_START, RD_END, instr->rd),
		FIELD(FUNCT_START, FUNCT_END, funct_code)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr->instr_bin, instr->instr_hex);
}

void build_rd_instruction(instruction *instr, int funct_code)
{
	instruction_field fields[] = {
		FIELD(RD_START, RD_END, instr->rd),
		FIELD(FUNCT_START, FUNCT_END, funct_code)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr->instr_bin, instr->instr_hex);
}

void build_rd_rs_instruction(instruction *instr, int funct_code)
{
	int rt_value = 0;
	if(instr->opcode == CLO) rt_value = CLO_RT;
	else if(instr->opcode == CLZ) rt_value = CLZ_RT;
	instruction_field fields[] = {
		FIELD(RS_START, RS_END, instr->rs),
		FIELD(RT_START, RT_END, rt_value),
		FIELD(RD_START, RD_END, instr->rd),
		FIELD(FUNCT_START, FUNCT_END, funct_code)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr->instr_bin, instr->instr_hex);
}

void build_rs_instruction(instruction *instr, int funct_code)
{
	instruction_field fields[] = {
		FIELD(RS_START, RS_END, instr->rs),
		FIELD(FUNCT_START, FUNCT_END, funct_code)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr->instr_bin, instr->instr_hex);
}

void build_rt_immediate_instruction(instruction *instr, int op_code)
{
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, op_code),
		FIELD(RT_START, RT_END, instr->rt),
		FIELD(IMM_START, IMM_END, instr->immediate)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr->instr_bin, instr->instr_hex);
	finalize_signed_value(&instr->immediate, instr);
}

void build_shift_instruction(instruction *instr, int funct_code)
{
	instruction_field fields[] = {
		FIELD(RT_START, RT_END, instr->rt),
		FIELD(RD_START, RD_END, instr->rd),
		FIELD(SHAMT_START, SHAMT_END, instr->sa),
		FIELD(FUNCT_START, FUNCT_END, funct_code)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr->instr_bin, instr->instr_hex);
	if(instr->opcode == ROTR) instr->instr_bin[ROTR_BIT] = 1;
	bin_arr_to_hex_arr(instr->instr_bin, instr->instr_hex);
}

void build_target_instruction(instruction *instr, int op_code)
{
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, op_code),
		FIELD(TARGET_START, TARGET_END, instr->target)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr->instr_bin, instr->instr_hex);
}

void build_variable_shift_instruction(instruction *instr, int funct_code)
{
	instruction_field fields[] = {
		FIELD(RS_START, RS_END, instr->rs),
		FIELD(RT_START, RT_END, instr->rt),
		FIELD(RD_START, RD_END, instr->rd),
		FIELD(FUNCT_START, FUNCT_END, funct_code)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr->instr_bin, instr->instr_hex);
	if(instr->opcode == ROTRV) instr->instr_bin[ROTRV_BIT] = 1;
	bin_arr_to_hex_arr(instr->instr_bin, instr->instr_hex);
}

void decode_branch_r1_operands(FILE *in, instruction *instr)
{
	char offset[16], rs[8];
	fscanf(in, " $%7[^,]  , %15s ", rs, offset);
	instr->offset = handle_sign(offset, instr);
	instr->rs = register_string_to_int(rs);
}

void decode_branch_r2_operands(FILE *in, instruction *instr)
{
	char offset[16], rs[8], rt[8];
	fscanf(in, " $%7[^,] , $%7[^,] , %15s ", rs, rt, offset);
	instr->offset = handle_sign(offset, instr);
	instr->rs = register_string_to_int(rs);
	instr->rt = register_string_to_int(rt);
}

void decode_memory_operands(FILE *in, instruction *instr)
{
	char base[8], offset[16], rt[8];
	fscanf(in, " $%7[^,] , %15[^(] ($%7[^)]) ", rt, offset, base);
	instr->base = register_string_to_int(base);
	instr->offset = handle_sign(offset, instr);
	instr->rt = register_string_to_int(rt);
}

void decode_r2_operands(FILE *in, instruction *instr)
{
	char rs[8], rt[8];
	fscanf(in, " $%7[^,] , $%7s ", rs, rt);
	instr->rs = register_string_to_int(rs);
	instr->rt = register_string_to_int(rt);
}

void decode_r3_immediate_operands(FILE *in, instruction *instr)
{
	char imm[16], rs[8], rt[8];
	fscanf(in, " $%7[^,] , $%7[^,] , %15s ", rt, rs, imm);
	instr->immediate = handle_sign(imm, instr);
	instr->rs = register_string_to_int(rs);
	instr->rt = register_string_to_int(rt);
}

void decode_r3_operands(FILE *in, instruction *instr)
{
	char rd[8], rs[8], rt[8];
	fscanf(in, " $%7[^,] , $%7[^,] , $%7s ", rd, rs, rt);
	instr->rd = register_string_to_int(rd);
	instr->rs = register_string_to_int(rs);
	instr->rt = register_string_to_int(rt);
}

void decode_rd_operand(FILE *in, instruction *instr)
{
	char rd[8];
	fscanf(in, " $%7s ", rd);
	instr->rd = register_string_to_int(rd);
}

void decode_rd_rs_operands(FILE *in, instruction *instr)
{
	char rd[8], rs[8];
	fscanf(in, " $%7[^,] , $%7s ", rd, rs);
	instr->rd = register_string_to_int(rd);
	instr->rs = register_string_to_int(rs);
}

void decode_rs_operand(FILE *in, instruction *instr)
{
	char rs[8];
	fscanf(in, " $%7s ", rs);
	instr->rs = register_string_to_int(rs);
}

void decode_rt_immediate_operands(FILE *in, instruction *instr)
{
	char imm[16], rt[8];
	fscanf(in, " $%7[^,] , %15s ", rt, imm);
	instr->immediate = handle_sign(imm, instr);
	instr->rt = register_string_to_int(rt);
}

void decode_shift_operands(FILE *in, instruction *instr)
{
	char rd[8], rt[8], sa[16];
	fscanf(in, " $%7[^,] , $%7[^,] , %15s ", rd, rt, sa);
	instr->rd = register_string_to_int(rd);
	instr->rt = register_string_to_int(rt);
	instr->sa = atoi(sa);
}

void decode_target_operands(FILE *in, instruction *instr)
{
	char target[16];
	fscanf(in, " %15s ", target);
	instr->target = atoi(target);
}

void decode_variable_shift_operands(FILE *in, instruction *instr)
{
	char rd[8], rs[8], rt[8];
	fscanf(in, " $%7[^,] , $%7[^,] , $%7s ", rd, rt, rs);
	instr->rd = register_string_to_int(rd);
	instr->rs = register_string_to_int(rs);
	instr->rt = register_string_to_int(rt);
}

instruction decode_instruction(int mode, FILE *fichier)
{
	char chunk[256];
	FILE *in = (!mode) ? stdin : fichier;
	instruction instr = {0};
	int dump;

	if(fscanf(in, "%255s", chunk) != 1)
	{
		instr.exit = 1;
		return instr;
	}

	instruction_desc *id = find_instruction(chunk);
	
	if(chunk[0] != '#')
	{
		if(chunk[0] == '.') printf("[!] Directives are currently not supported.\n");
		else if(id)
		{
			instr.opcode = id->op;
			switch(id->format)
			{
				case BRANCH_R1 :
					decode_branch_r1_operands(in, &instr);
					build_branch_r1_instruction(&instr, id->code);
					snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, %d -> 0x%s\n", id->mnemonic, instr.rs, instr.offset, instr.instr_hex);
					break;
				case BRANCH_R2 :
					decode_branch_r2_operands(in, &instr);
					build_branch_r2_instruction(&instr, id->code);
					snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $%d, %d -> 0x%s\n", id->mnemonic, instr.rs, instr.rt, instr.offset, instr.instr_hex);
					break;
				case CMD_EXIT :
					instr.exit = 1;
					break;
				case CMD_NOP :
					bin_arr_to_hex_arr(instr.instr_bin, instr.instr_hex);
					snprintf(instr.to_string, sizeof(instr.to_string), "NOP -> 0x%s\n", instr.instr_hex);
					break;
				case MEMORY :
					decode_memory_operands(in, &instr);
					build_memory_instruction(&instr, id->code);
					snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, %d($%d) -> 0x%s\n", id->mnemonic, instr.rt, instr.offset, instr.base, instr.instr_hex);
					break;
				case R2 :
					decode_r2_operands(in, &instr);
					build_r2_instruction(&instr, id->code);
					snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $%d -> 0x%s\n", id->mnemonic, instr.rs, instr.rt, instr.instr_hex);
					break;
				case R3_IMMEDIATE :
					decode_r3_immediate_operands(in, &instr);
					build_r3_immediate_instruction(&instr, id->code);
					snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $%d, %d -> 0x%s\n", id->mnemonic, instr.rt, instr.rs, instr.immediate, instr.instr_hex);
					break;
				case R3 :
					decode_r3_operands(in, &instr);
					build_r3_instruction(&instr, id->code);
					snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $%d, $%d -> 0x%s\n", id->mnemonic, instr.rd, instr.rs, instr.rt, instr.instr_hex);
					break;
				case RD :
					decode_rd_operand(in, &instr);
					build_rd_instruction(&instr, id->code);
					snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d -> 0x%s\n", id->mnemonic, instr.rd, instr.instr_hex);
					break;
				case RD_RS :
					decode_rd_rs_operands(in, &instr);
					build_rd_rs_instruction(&instr, id->code);
					snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $%d -> 0x%s\n", id->mnemonic, instr.rd, instr.rs, instr.instr_hex);
					break;
				case RS :
					decode_rs_operand(in, &instr);
					build_rs_instruction(&instr, id->code);
					snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d -> 0x%s\n", id->mnemonic, instr.rs, instr.instr_hex);
					break;
				case RT_IMMEDIATE :
					decode_rt_immediate_operands(in, &instr);
					build_rt_immediate_instruction(&instr, id->code);
					snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, %d -> 0x%s\n", id->mnemonic, instr.rt, instr.immediate, instr.instr_hex);
					break;
				case SHIFT :
					decode_shift_operands(in, &instr);
					build_shift_instruction(&instr, id->code);
					snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $%d, %d -> 0x%s\n", id->mnemonic, instr.rd, instr.rt, instr.sa, instr.instr_hex);
					break;
				case TARGET :
					decode_target_operands(in, &instr);
					build_target_instruction(&instr, id->code);
					snprintf(instr.to_string, sizeof(instr.to_string), "%s %d -> 0x%s\n", id->mnemonic, instr.target, instr.instr_hex);
					break;
				case VARIABLE_SHIFT :
					decode_variable_shift_operands(in, &instr);
					build_variable_shift_instruction(&instr, id->code);
					snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $%d, $%d -> 0x%s\n", id->mnemonic, instr.rd, instr.rt, instr.rs, instr.instr_hex);
					break;
			}
			log_instruction(&instr);
		}
	}
	else
		while((dump = fgetc(in)) != '\n' && dump != EOF);

	return(instr);
}