#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decode.h"
#include "instruction.h"
#include "utils.h"

void build_instruction_bin(instruction_field *arr, size_t field_count, instruction *instr)
{
	for(size_t i = 0; i < field_count; i++) long_to_bin_arr(arr[i].start_bit, arr[i].end_bit, arr[i].value, instr->instr_bin);
	bin_arr_to_hex_arr(instr->instr_bin, instr->instr_hex);
}

int32_t handle_sign(char *param, instruction *instr)
{
	if(is_negative(param))
	{
		instr->negative = 1;
		remove_sign(param);
	}

	return((int32_t) atoi(param));
}

void finalize_signed_value(int32_t *value, instruction *instr, uint8_t start_bit, uint8_t end_bit)
{
	if(instr->negative)
	{
		*value *= -1;
		bin_twos_complement(start_bit, end_bit, instr->instr_bin);
		bin_arr_to_hex_arr(instr->instr_bin, instr->instr_hex);
	}
}

void build_offset_16_instruction(instruction *instr, int op_code)
{
	int32_t rt_value = 0;
	if(instr->opcode == BAL) rt_value = BAL_RT;
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, op_code),
		FIELD(RT_START, RT_END, rt_value),
		FIELD(OFFSET_16_START, OFFSET_END, instr->offset)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr);
	finalize_signed_value(&instr->offset, instr, OFFSET_16_START, OFFSET_END);
	instr->offset = sign_extend(instr->offset, OFFSET_16_SIZE);
}

void build_offset_26_instruction(instruction *instr, int op_code)
{
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, op_code),
		FIELD(OFFSET_26_START, OFFSET_END, instr->offset)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr);
	finalize_signed_value(&instr->offset, instr, OFFSET_26_START, OFFSET_END);
	instr->offset = sign_extend(instr->offset, OFFSET_26_SIZE);
}

void build_rd_instruction(instruction *instr, int funct_code)
{
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, OPCODE_SPECIAL),
		FIELD(RD_START, RD_END, instr->rd),
		FIELD(FUNCT_START, FUNCT_END, funct_code)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr);
}

void build_rd_rs_instruction(instruction *instr, int funct_code)
{
	int32_t sa_value = 0;
	if(instr->opcode == CLO || instr->opcode == CLZ) sa_value = CLO_CLZ_SA;
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, OPCODE_SPECIAL),
		FIELD(RS_START, RS_END, instr->rs),
		FIELD(RD_START, RD_END, instr->rd),
		FIELD(SHAMT_START, SHAMT_END, sa_value),
		FIELD(FUNCT_START, FUNCT_END, funct_code)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr);
}

void build_rd_rs_rt_instruction(instruction *instr, int funct_code)
{
	uint8_t shamt_value = 0;
	if(instr->opcode == DIV || instr->opcode == DIVU) shamt_value = DIV_DIVU_SA;
	else if(instr->opcode == MOD || instr->opcode == MODU) shamt_value = MOD_MODU_SA;
	else if(instr->opcode == MUH || instr->opcode == MUHU) shamt_value = MUH_MUHU_SA;
	else if(instr->opcode == MUL || instr->opcode == MULU) shamt_value = MUL_MULU_SA;
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, OPCODE_SPECIAL),
		FIELD(RS_START, RS_END, instr->rs),
		FIELD(RT_START, RT_END, instr->rt),
		FIELD(RD_START, RD_END, instr->rd),
		FIELD(SHAMT_START, SHAMT_END, shamt_value),
		FIELD(FUNCT_START, FUNCT_END, funct_code)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr);
}

void build_rd_rt_rs_instruction(instruction *instr, int funct_code)
{
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, OPCODE_SPECIAL),
		FIELD(RS_START, RS_END, instr->rs),
		FIELD(RT_START, RT_END, instr->rt),
		FIELD(RD_START, RD_END, instr->rd),
		FIELD(FUNCT_START, FUNCT_END, funct_code)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr);
	if(instr->opcode == ROTRV) instr->instr_bin[ROTRV_BIT] = 1;
	bin_arr_to_hex_arr(instr->instr_bin, instr->instr_hex);
}

void build_rd_rt_sa_instruction(instruction *instr, int funct_code)
{
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, OPCODE_SPECIAL),
		FIELD(RT_START, RT_END, instr->rt),
		FIELD(RD_START, RD_END, instr->rd),
		FIELD(SHAMT_START, SHAMT_END, instr->sa),
		FIELD(FUNCT_START, FUNCT_END, funct_code)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr);
	if(instr->opcode == ROTR) instr->instr_bin[ROTR_BIT] = 1;
	bin_arr_to_hex_arr(instr->instr_bin, instr->instr_hex);
}

void build_rs_instruction(instruction *instr, int funct_code)
{
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, OPCODE_SPECIAL),
		FIELD(RS_START, RS_END, instr->rs),
		FIELD(FUNCT_START, FUNCT_END, funct_code)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr);
}

void build_rs_immediate_instruction(instruction *instr, int op_code)
{
	int32_t imm_value = instr->immediate, rt_value = 0;
	if(instr->opcode == AUIPC) rt_value = AUIPC_RT;
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, op_code),
		FIELD(RS_START, RS_END, instr->rs),
		FIELD(RT_START, RT_END, rt_value),
		FIELD(IMM_START, IMM_END, instr->immediate)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr);
	finalize_signed_value(&imm_value, instr, IMM_START, IMM_END);
	instr->immediate = (int16_t) imm_value;
}

void build_rs_offset_16_instruction(instruction *instr, int op_code)
{
	int32_t rt_value = 0;
	if(instr->opcode == BGEZ) rt_value = BGEZ_RT;
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, op_code),
		FIELD(RS_START, RS_END, instr->rs),
		FIELD(RT_START, RT_END, rt_value),
		FIELD(OFFSET_16_START, OFFSET_END, instr->offset)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr);
	finalize_signed_value(&instr->offset, instr, OFFSET_16_START, OFFSET_END);
	instr->offset = sign_extend(instr->offset, OFFSET_16_SIZE);
}

void build_rs_offset_21_instruction(instruction *instr, int op_code)
{
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, op_code),
		FIELD(RS_START, RS_END, instr->rs),
		FIELD(OFFSET_21_START, IMM_END, instr->offset)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr);
	finalize_signed_value(&instr->offset, instr, OFFSET_21_START, OFFSET_END);
	instr->offset = sign_extend(instr->offset, OFFSET_21_SIZE);
}

void build_rs_rt_instruction(instruction *instr, int funct_code)
{
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, OPCODE_SPECIAL),
		FIELD(RS_START, RS_END, instr->rs),
		FIELD(RT_START, RT_END, instr->rt),
		FIELD(FUNCT_START, FUNCT_END, funct_code)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr);
}

void build_rs_rt_offset_16_instruction(instruction *instr, int op_code)
{
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, op_code),
		FIELD(RS_START, RS_END, instr->rs),
		FIELD(RT_START, RT_END, instr->rt),
		FIELD(OFFSET_16_START, OFFSET_END, instr->offset)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr);
	finalize_signed_value(&instr->offset, instr, OFFSET_16_START, OFFSET_END);
	instr->offset = sign_extend(instr->offset, OFFSET_16_SIZE);
}

void build_rt_immediate_instruction(instruction *instr, int op_code)
{
	int32_t imm_value = instr->immediate;
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, op_code),
		FIELD(RT_START, RT_END, instr->rt),
		FIELD(IMM_START, IMM_END, instr->immediate)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr);
	finalize_signed_value(&imm_value, instr, IMM_START, IMM_END);
	instr->immediate = (int16_t) imm_value;
}

void build_rt_offset_16_instruction(instruction *instr, int op_code)
{
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, op_code),
		FIELD(RT_START, RT_END, instr->rt),
		FIELD(OFFSET_16_START, OFFSET_END, instr->offset)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr);
	finalize_signed_value(&instr->offset, instr, OFFSET_16_START, OFFSET_END);
	instr->offset = sign_extend(instr->offset, OFFSET_16_SIZE);
}

void build_rt_offset_base_instruction(instruction *instr, int op_code)
{
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, op_code),
		FIELD(BASE_START, BASE_END, instr->base),
		FIELD(RT_START, RT_END, instr->rt),
		FIELD(OFFSET_16_START, OFFSET_END, instr->offset)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr);
	finalize_signed_value(&instr->offset, instr, OFFSET_16_START, OFFSET_END);
	instr->offset = sign_extend(instr->offset, OFFSET_16_SIZE);
}

void build_rt_rs_immediate_instruction(instruction *instr, int op_code)
{
	int32_t imm_value = instr->immediate;
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, op_code),
		FIELD(RS_START, RS_END, instr->rs),
		FIELD(RT_START, RT_END, instr->rt),
		FIELD(IMM_START, IMM_END, instr->immediate)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr);
	finalize_signed_value(&imm_value, instr, IMM_START, IMM_END);
	instr->immediate = (int16_t) imm_value;
}

void build_target_instruction(instruction *instr, int op_code)
{
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, op_code),
		FIELD(TARGET_START, TARGET_END, instr->target)};
	build_instruction_bin(fields, ARR_SIZE(fields), instr);
}

void decode_offset_operand(char *line, instruction *instr)
{
	char offset[16];
	sscanf(line, " %15s ", offset);
	instr->offset = handle_sign(offset, instr);
}

void decode_rd_operand(char *line, instruction *instr)
{
	char rd[8];
	sscanf(line, " $%7s ", rd);
	instr->rd = register_string_to_int(rd);
}

void decode_rd_rs_operands(char *line, instruction *instr)
{
	char rd[8], rs[8];
	sscanf(line, " $%7[^,] , $%7s ", rd, rs);
	instr->rd = register_string_to_int(rd);
	instr->rs = register_string_to_int(rs);
}

void decode_rd_rs_rt_operands(char *line, instruction *instr)
{
	char rd[8], rs[8], rt[8];
	sscanf(line, " $%7[^,] , $%7[^,] , $%7s ", rd, rs, rt);
	instr->rd = register_string_to_int(rd);
	instr->rs = register_string_to_int(rs);
	instr->rt = register_string_to_int(rt);
}

void decode_rd_rt_rs_operands(char *line, instruction *instr)
{
	char rd[8], rs[8], rt[8];
	sscanf(line, " $%7[^,] , $%7[^,] , $%7s ", rd, rt, rs);
	instr->rd = register_string_to_int(rd);
	instr->rs = register_string_to_int(rs);
	instr->rt = register_string_to_int(rt);
}

void decode_rd_rt_sa_operands(char *line, instruction *instr)
{
	char rd[8], rt[8], sa[16];
	sscanf(line, " $%7[^,] , $%7[^,] , %15s ", rd, rt, sa);
	instr->rd = register_string_to_int(rd);
	instr->rt = register_string_to_int(rt);
	instr->sa = (uint8_t) atoi(sa);
}

void decode_rs_operand(char *line, instruction *instr)
{
	char rs[8];
	sscanf(line, " $%7s ", rs);
	instr->rs = register_string_to_int(rs);
}

void decode_rs_immediate_operands(char *line, instruction *instr)
{
	char imm[16], rs[8];
	sscanf(line, " $%7[^,], %15s ", rs, imm);
	instr->immediate = (int16_t) handle_sign(imm, instr);
	instr->rs = register_string_to_int(rs);
}

void decode_rs_offset_operands(char *line, instruction *instr)
{
	char offset[16], rs[8];
	sscanf(line, " $%7[^,]  , %15s ", rs, offset);
	instr->offset = handle_sign(offset, instr);
	instr->rs = register_string_to_int(rs);
}

void decode_rs_rt_operands(char *line, instruction *instr)
{
	char rs[8], rt[8];
	sscanf(line, " $%7[^,] , $%7s ", rs, rt);
	instr->rs = register_string_to_int(rs);
	instr->rt = register_string_to_int(rt);
}

void decode_rs_rt_offset_operands(char *line, instruction *instr)
{
	char offset[16], rs[8], rt[8];
	sscanf(line, " $%7[^,] , $%7[^,] , %15s ", rs, rt, offset);
	instr->offset = handle_sign(offset, instr);
	instr->rs = register_string_to_int(rs);
	instr->rt = register_string_to_int(rt);
}

void decode_rt_immediate_operands(char *line, instruction *instr)
{
	char imm[16], rt[8];
	sscanf(line, " $%7[^,] , %15s ", rt, imm);
	instr->immediate = (int16_t) handle_sign(imm, instr);
	instr->rt = register_string_to_int(rt);
}

void decode_rt_offset_operands(char *line, instruction *instr)
{
	char offset[16], rt[8];
	sscanf(line, " $%7[^,] , %15s ", rt, offset);
	instr->offset = handle_sign(offset, instr);
	instr->rt = register_string_to_int(rt);
}

void decode_rt_offset_base_operands(char *line, instruction *instr)
{
	char base[8], offset[16], rt[8];
	sscanf(line, " $%7[^,] , %15[^(] ($%7[^)]) ", rt, offset, base);
	instr->base = register_string_to_int(base);
	instr->offset = handle_sign(offset, instr);
	instr->rt = register_string_to_int(rt);
}

void decode_rt_rs_immediate_operands(char *line, instruction *instr)
{
	char imm[16], rs[8], rt[8];
	sscanf(line, " $%7[^,] , $%7[^,] , %15s ", rt, rs, imm);
	instr->immediate = (int16_t) handle_sign(imm, instr);
	instr->rs = register_string_to_int(rs);
	instr->rt = register_string_to_int(rt);
}

void decode_rt_rs_offset_operands(char *line, instruction *instr)
{
	char offset[16], rs[8], rt[8];
	sscanf(line, " $%7[^,] , $%7[^,] , %15s ", rt, rs, offset);
	instr->offset = handle_sign(offset, instr);
	instr->rs = register_string_to_int(rs);
	instr->rt = register_string_to_int(rt);
}

void decode_target_operands(char *line, instruction *instr)
{
	char target[16];
	sscanf(line, " %15s ", target);
	instr->target = atoi(target);
}

instruction decode_instruction(int mode, FILE *fichier)
{
	char mnemonic[16], *line, *ptr;
	FILE *in = (!mode) ? stdin : fichier;
	instruction instr = {0};
	int consumed = 0;

	line = read_full_line(in);
	if(!line)
	{
		instr.exit = 1;
		return instr;
	}
	line[strcspn(line, "\n")] = '\0';

	ptr = line;
	while(*ptr == ' ' || *ptr == '\t') ptr++;
	if(*ptr == '#' || *ptr == '\0')
	{
		free(line);
		return(instr);
	}

	sscanf(ptr, "%15s%n", mnemonic, &consumed);
	ptr += consumed;

	const instruction_desc *id = find_instruction(mnemonic);
	
	if(id)
	{
		// if(chunk[0] == '.') printf("[!] Directives are currently not supported.\n");
		instr.opcode = id->op;
		switch(id->format)
		{
			case CMD_EXIT :
				instr.exit = 1;
				break;
			case CMD_NOP :
				bin_arr_to_hex_arr(instr.instr_bin, instr.instr_hex);
				snprintf(instr.to_string, sizeof(instr.to_string), "NOP -> 0x%s\n", instr.instr_hex);
				break;
			case OFFSET_16 :
				decode_offset_operand(ptr, &instr);
				build_offset_16_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s %d -> 0x%s\n", id->mnemonic, instr.offset, instr.instr_hex);
				break;
			case OFFSET_26 :
				decode_offset_operand(ptr, &instr);
				build_offset_26_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s %d -> 0x%s\n", id->mnemonic, instr.offset, instr.instr_hex);
				break;
			case RD :
				decode_rd_operand(ptr, &instr);
				build_rd_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d -> 0x%s\n", id->mnemonic, instr.rd, instr.instr_hex);
				break;
			case RD_RS :
				decode_rd_rs_operands(ptr, &instr);
				build_rd_rs_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $%d -> 0x%s\n", id->mnemonic, instr.rd, instr.rs, instr.instr_hex);
				break;
			case RD_RS_RT :
				decode_rd_rs_rt_operands(ptr, &instr);
				build_rd_rs_rt_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $%d, $%d -> 0x%s\n", id->mnemonic, instr.rd, instr.rs, instr.rt, instr.instr_hex);
				break;
			case RD_RT_RS :
				decode_rd_rt_rs_operands(ptr, &instr);
				build_rd_rt_rs_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $%d, $%d -> 0x%s\n", id->mnemonic, instr.rd, instr.rt, instr.rs, instr.instr_hex);
				break;
			case RD_RT_SA :
				decode_rd_rt_sa_operands(ptr, &instr);
				build_rd_rt_sa_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $%d, %d -> 0x%s\n", id->mnemonic, instr.rd, instr.rt, instr.sa, instr.instr_hex);
				break;
			case RS :
				decode_rs_operand(ptr, &instr);
				build_rs_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d -> 0x%s\n", id->mnemonic, instr.rs, instr.instr_hex);
				break;
			case RS_IMMEDIATE :
				decode_rs_immediate_operands(ptr, &instr);
				build_rs_immediate_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, %d -> 0x%s\n", id->mnemonic, instr.rs, instr.immediate, instr.instr_hex);
				break;
			case RS_OFFSET_16 :
				decode_rs_offset_operands(ptr, &instr);
				build_rs_offset_16_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, %d -> 0x%s\n", id->mnemonic, instr.rs, instr.offset, instr.instr_hex);
				break;
			case RS_OFFSET_21 :
				decode_rs_offset_operands(ptr, &instr);
				build_rs_offset_21_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, %d -> 0x%s\n", id->mnemonic, instr.rs, instr.offset, instr.instr_hex);
				break;
			case RS_RT :
				decode_rs_rt_operands(ptr, &instr);
				build_rs_rt_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $%d -> 0x%s\n", id->mnemonic, instr.rs, instr.rt, instr.instr_hex);
				break;
			case RS_RT_OFFSET_16 :
				decode_rs_rt_offset_operands(ptr, &instr);
				build_rs_rt_offset_16_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $%d, %d -> 0x%s\n", id->mnemonic, instr.rs, instr.rt, instr.offset, instr.instr_hex);
				break;
			case RT_IMMEDIATE :
				decode_rt_immediate_operands(ptr, &instr);
				build_rt_immediate_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, %d -> 0x%s\n", id->mnemonic, instr.rt, instr.immediate, instr.instr_hex);
				break;
			case RT_OFFSET_16 :
				decode_rt_offset_operands(ptr, &instr);
				build_rt_offset_16_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s %d, %d -> 0x%s\n", id->mnemonic, instr.rt, instr.offset, instr.instr_hex);
				break;
			case RT_OFFSET_BASE :
				decode_rt_offset_base_operands(ptr, &instr);
				build_rt_offset_base_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, %d($%d) -> 0x%s\n", id->mnemonic, instr.rt, instr.offset, instr.base, instr.instr_hex);
				break;
			case RT_RS_IMMEDIATE :
				decode_rt_rs_immediate_operands(ptr, &instr);
				build_rt_rs_immediate_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $%d, %d -> 0x%s\n", id->mnemonic, instr.rt, instr.rs, instr.immediate, instr.instr_hex);
				break;
			case RT_RS_OFFSET_16 :
				decode_rt_rs_offset_operands(ptr, &instr);
				build_rs_rt_offset_16_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $%d, %d -> 0x%s\n", id->mnemonic, instr.rt, instr.rs, instr.offset, instr.instr_hex);
				break;
			case TARGET :
				decode_target_operands(ptr, &instr);
				build_target_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s %d -> 0x%s\n", id->mnemonic, instr.target, instr.instr_hex);
				break;
		}
		log_instruction(&instr);
	}

	free(line);

	return(instr);
}