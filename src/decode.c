#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decode.h"
#include "instruction.h"
#include "utils.h"

void build_instruction_bin(instruction_field *arr, size_t field_count, int *bin_arr, char *hex_arr)
{
	for(size_t i = 0; i < field_count; i++)
		long_to_bin_arr(arr[i].start_bit, arr[i].end_bit, arr[i].value, bin_arr);
	bin_arr_to_hex_arr(bin_arr, hex_arr);
}

int handle_sign(char *param, int *negative)
{
	*negative = 0;

	if(is_negative(param))
	{
		*negative = 1;
		remove_sign(param);
	}

	return(atoi(param));
}

void finalize_signed_value(int *value, int negative, int *bin_arr, char *hex_arr)
{
	if(negative)
	{
		*value *= -1;
		bin_twos_complement(IMM_START, IMM_END, bin_arr);
		bin_arr_to_hex_arr(bin_arr, hex_arr);
	}
}

void build_branch_r1_instruction(instruction *instr, int op_code, int *bin_arr, int negative)
{
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, op_code),
		FIELD(RS_START, RS_END, instr->rs),
		FIELD(IMM_START, IMM_END, instr->offset)};
	build_instruction_bin(fields, 3, bin_arr, instr->instr_hex);
	finalize_signed_value(&instr->offset, negative, bin_arr, instr->instr_hex);
}

void build_branch_r2_instruction(instruction *instr, int op_code, int *bin_arr, int negative)
{
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, op_code),
		FIELD(RS_START, RS_END, instr->rs),
		FIELD(RT_START, RT_END, instr->rt),
		FIELD(IMM_START, IMM_END, instr->offset)};
	build_instruction_bin(fields, 4, bin_arr, instr->instr_hex);
	finalize_signed_value(&instr->offset, negative, bin_arr, instr->instr_hex);
}

void build_memory_instruction(instruction *instr, int op_code, int *bin_arr, int negative)
{
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, op_code),
		FIELD(RS_START, RS_END, instr->base),
		FIELD(RT_START, RT_END, instr->rt),
		FIELD(IMM_START, IMM_END, instr->offset)};
	build_instruction_bin(fields, 4, bin_arr, instr->instr_hex);
	finalize_signed_value(&instr->offset, negative, bin_arr, instr->instr_hex);
}

void build_r2_instruction(instruction *instr, int funct_code, int *bin_arr)
{
	instruction_field fields[] = {
		FIELD(RS_START, RS_END, instr->rs),
		FIELD(RT_START, RT_END, instr->rt),
		FIELD(FUNCT_START, FUNCT_END, funct_code)};
	build_instruction_bin(fields, 3, bin_arr, instr->instr_hex);
}

void build_r3_instruction(instruction *instr, int funct_code, int *bin_arr)
{
	instruction_field fields[] = {
		FIELD(RS_START, RS_END, instr->rs),
		FIELD(RT_START, RT_END, instr->rt),
		FIELD(RD_START, RD_END, instr->rd),
		FIELD(FUNCT_START, FUNCT_END, funct_code)};
	build_instruction_bin(fields, 4, bin_arr, instr->instr_hex);
}

void build_rd_instruction(instruction *instr, int funct_code, int *bin_arr)
{
	instruction_field fields[] = {
		FIELD(RD_START, RD_END, instr->rd),
		FIELD(FUNCT_START, FUNCT_END, funct_code)};
	build_instruction_bin(fields, 2, bin_arr, instr->instr_hex);
}

void build_shift_instruction(instruction *instr, int funct_code, int rotr, int *bin_arr)
{
	instruction_field fields[] = {
		FIELD(RT_START, RT_END, instr->rt),
		FIELD(RD_START, RD_END, instr->rd),
		FIELD(SHAMT_START, SHAMT_END, instr->sa),
		FIELD(FUNCT_START, FUNCT_END, funct_code)};
	build_instruction_bin(fields, 4, bin_arr, instr->instr_hex);
	if(rotr)
	{
		bin_arr[ROTR_BIT] = 1;
		bin_arr_to_hex_arr(bin_arr, instr->instr_hex);
	}
}

void build_target_instruction(instruction *instr, int op_code, int *bin_arr)
{
	instruction_field fields[] = {
		FIELD(OPCODE_START, OPCODE_END, op_code),
		FIELD(TARGET_START, TARGET_END, instr->target)};
	build_instruction_bin(fields, 2, bin_arr, instr->instr_hex);
}

void decode_branch_r1_operands(FILE *in, instruction *instr, int *negative)
{
	char offset[16], rs[8];
	fscanf(in, " $%[^,]  , %s ", rs, offset);
	instr->offset = handle_sign(offset, negative);
	instr->rs = register_string_to_int(rs);
}

void decode_branch_r2_operands(FILE *in, instruction *instr, int *negative)
{
	char offset[16], rs[8], rt[8];
	fscanf(in, " $%[^,] , $%[^,] , %s ", rs, rt, offset);
	instr->offset = handle_sign(offset, negative);
	instr->rs = register_string_to_int(rs);
	instr->rt = register_string_to_int(rt);
}

void decode_memory_operands(FILE *in, instruction *instr, int *negative)
{
	char base[8], offset[16], rt[8];
	fscanf(in, " $%[^,] , %[^(] ($%[^)]) ", rt, offset, base);
	instr->base = register_string_to_int(base);
	instr->offset = handle_sign(offset, negative);
	instr->rt = register_string_to_int(rt);
}

void decode_r2_operands(FILE *in, instruction *instr)
{
	char rs[8], rt[8];
	fscanf(in, " $%[^,] , $%s ", rs, rt);
	instr->rs = register_string_to_int(rs);
	instr->rt = register_string_to_int(rt);
}

void decode_r3_operands(FILE *in, instruction *instr)
{
	char rd[8], rs[8], rt[8];
	fscanf(in, " $%[^,] , $%[^,] , $%s ", rd, rs, rt);
	instr->rd = register_string_to_int(rd);
	instr->rs = register_string_to_int(rs);
	instr->rt = register_string_to_int(rt);
}

void decode_rd_operand(FILE *in, instruction *instr)
{
	char rd[8];
	fscanf(in, " $%s ", rd);
	instr->rd = register_string_to_int(rd);
}

void decode_rs_operand(FILE *in, instruction *instr)
{
	char rs[8];
	fscanf(in, " $%s ", rs);
	instr->rs = register_string_to_int(rs);
}

void decode_shift_operands(FILE *in, instruction *instr)
{
	char rd[8], rt[8], sa[16];
	fscanf(in, " $%[^,] , $%[^,] , %s ", rd, rt, sa);
	instr->rd = register_string_to_int(rd);
	instr->rt = register_string_to_int(rt);
	instr->sa = atoi(sa);
}

void decode_target_operands(FILE *in, instruction *instr)
{
	char target[16];
	fscanf(in, " %s ", target);
	instr->target = atoi(target);
}

instruction decode_instruction(int mode, FILE *fichier)
{
	char chunk[50], instr_hex[9], param1[20], param2[20], param3[20];
	int instr_bin[32] = {0}, negative = 0;
	instruction instr = {0};

	FILE *in = (!mode) ? stdin : fichier;

	if(fscanf(in, "%49s", chunk) != 1)
	{
		instr.exit = 1;
		return instr;
	}
	
	if(chunk[0] != '#')
	{
		if(chunk[0] == '.')
			printf("[!] Directives are currently not supported.\n");
		else
		{
			if(!strcmp(chunk, "ADD"))
			{
				instr.opcode = ADD;
				decode_r3_operands(in, &instr);
				build_r3_instruction(&instr, FUNCT_ADD, instr_bin);
				sprintf(instr.toString, "ADD $%d,$%d,$%d -> 0x%s\n", instr.rd, instr.rs, instr.rt, instr_hex);
			}
			else if(!strcmp(chunk, "ADDI"))
			{
				fscanf(in, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				instr.rt = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.immediate = handle_sign(param3, &negative);
				instruction_field fields[] = {FIELD(0, 5, 8), FIELD(6, 10, instr.rs), FIELD(11, 15, instr.rt), FIELD(16, 31, instr.immediate)};
				build_instruction_bin(fields, 4, instr_bin, instr_hex);
				if(negative)
				{
					instr.immediate *= -1;
					bin_twos_complement(16, 31, instr_bin);
				}
				bin_arr_to_hex_arr(instr_bin, instr_hex); // TODO: to keep for now because if(negative) is after build_instruction_bin()
				strcpy(instr.instr_hex, instr_hex);
				instr.opcode = ADDI;
				sprintf(instr.toString, "ADDI $%s,$%s,%d -> 0x%s\n", param1, param2, instr.immediate, instr_hex);
			}
			else if(!strcmp(chunk, "AND"))
			{
				instr.opcode = AND;
				decode_r3_operands(in, &instr);
				build_r3_instruction(&instr, FUNCT_AND, instr_bin);
				sprintf(instr.toString, "AND $%d,$%d,$%d -> 0x%s\n", instr.rd, instr.rs, instr.rt, instr_hex);
			}
			else if(!strcmp(chunk, "BEQ"))
			{
				instr.opcode = BEQ;
				decode_branch_r2_operands(in, &instr, &negative);
				build_branch_r2_instruction(&instr, OPCODE_BEQ, instr_bin, negative);
				sprintf(instr.toString, "BEQ $%d,$%d,%d -> 0x%s\n", instr.rs, instr.rt, instr.offset, instr_hex);
			}
			else if(!strcmp(chunk, "BGTZ"))
			{
				instr.opcode = BGTZ;
				decode_branch_r1_operands(in, &instr, &negative);
				build_branch_r1_instruction(&instr, OPCODE_BGTZ, instr_bin, negative);
				sprintf(instr.toString, "BGTZ $%d,%d -> 0x%s\n", instr.rs, instr.offset, instr_hex);
			}
			else if(!strcmp(chunk, "BLEZ"))
			{
				instr.opcode = BLEZ;
				decode_branch_r1_operands(in, &instr, &negative);
				build_branch_r1_instruction(&instr, OPCODE_BLEZ, instr_bin, negative);
				sprintf(instr.toString, "BLEZ $%d,%d -> 0x%s\n", instr.rs, instr.offset, instr_hex);
			}
			else if(!strcmp(chunk, "BNE"))
			{
				instr.opcode = BNE;
				decode_branch_r2_operands(in, &instr, &negative);
				build_branch_r2_instruction(&instr, OPCODE_BNE, instr_bin, negative);
				sprintf(instr.toString, "BNE $%d,$%d,%d -> 0x%s\n", instr.rs, instr.rt, instr.offset, instr_hex);
			}
			else if(!strcmp(chunk, "DIV"))
			{
				instr.opcode = DIV;
				decode_r2_operands(in, &instr);
				build_r2_instruction(&instr, FUNCT_DIV, instr_bin);
				sprintf(instr.toString, "DIV $%d,$%d -> 0x%s\n", instr.rs, instr.rt, instr_hex);
			}
			else if(!strcmp(chunk, "EXIT"))
				instr.exit = 1;
			else if(!strcmp(chunk, "J"))
			{
				instr.opcode = J;
				decode_target_operands(in, &instr);
				build_target_instruction(&instr, OPCODE_J, instr_bin);
				sprintf(instr.toString, "J %d -> 0x%s\n", instr.target, instr_hex);
			}
			else if(!strcmp(chunk, "JAL"))
			{
				instr.opcode = JAL;
				decode_target_operands(in, &instr);
				build_target_instruction(&instr, OPCODE_JAL, instr_bin);
				sprintf(instr.toString, "JAL %d -> 0x%s\n", instr.target, instr_hex);
			}
			else if(!strcmp(chunk, "JR"))
			{
				decode_rs_operand(in, &instr);
				instruction_field fields[] = {FIELD(6, 10, instr.rs), FIELD(11, 31, 8)};
				build_instruction_bin(fields, 2, instr_bin, instr_hex);
				strcpy(instr.instr_hex, instr_hex);
				instr.opcode = JR;
				sprintf(instr.toString, "JR $%s -> 0x%s\n", param1, instr_hex);
			}
			else if(!strcmp(chunk, "LUI"))
			{
				fscanf(in, " $%[^,] , %s ", param1, param2);
				instr.rt = register_string_to_int(param1);
				instr.immediate = handle_sign(param2, &negative);
				instruction_field fields[] = {FIELD(0, 5, 15), FIELD(11, 15, instr.rt), FIELD(16, 31, instr.immediate)};
				build_instruction_bin(fields, 3, instr_bin, instr_hex);
				if(negative)
				{
					instr.immediate *= -1;
					bin_twos_complement(16, 31, instr_bin);
				}
				bin_arr_to_hex_arr(instr_bin, instr_hex); // TODO: to keep for now because if(negative) is after build_instruction_bin()
				strcpy(instr.instr_hex, instr_hex);
				instr.opcode = LUI;
				sprintf(instr.toString, "LUI $%s,%d -> 0x%s\n", param1, instr.immediate, instr_hex);
			}
			else if(!strcmp(chunk, "LW"))
			{
				instr.opcode = LW;
				decode_memory_operands(in, &instr, &negative);
				build_memory_instruction(&instr, OPCODE_LW, instr_bin, negative);
				sprintf(instr.toString, "LW $%d,%d($%d) -> 0x%s\n", instr.rt, instr.offset, instr.base, instr_hex);
			}
			else if(!strcmp(chunk, "MFHI"))
			{
				instr.opcode = MFHI;
				decode_rd_operand(in, &instr);
				build_rd_instruction(&instr, FUNCT_MFHI, instr_bin);
				sprintf(instr.toString, "MFHI $%d -> 0x%s\n", instr.rd, instr_hex);
			}
			else if(!strcmp(chunk, "MFLO"))
			{
				instr.opcode = MFLO;
				decode_rd_operand(in, &instr);
				build_rd_instruction(&instr, FUNCT_MFLO, instr_bin);
				sprintf(instr.toString, "MFLO $%d -> 0x%s\n", instr.rd, instr_hex);
			}
			else if(!strcmp(chunk, "MULT"))
			{
				instr.opcode = MULT;
				decode_r2_operands(in, &instr);
				build_r2_instruction(&instr, FUNCT_MULT, instr_bin);
				sprintf(instr.toString, "MULT $%d,$%d -> 0x%s\n", instr.rs, instr.rt, instr_hex);
			}
			else if(!strcmp(chunk, "NOP"))
			{
				instr.opcode = NOP;
				bin_arr_to_hex_arr(instr_bin, instr.instr_hex);
				sprintf(instr.toString, "NOP -> 0x%s\n", instr_hex);
			}
			else if(!strcmp(chunk, "OR"))
			{
				instr.opcode = OR;
				decode_r3_operands(in, &instr);
				build_r3_instruction(&instr, FUNCT_OR, instr_bin);
				sprintf(instr.toString, "OR $%d,$%d,$%d -> 0x%s\n", instr.rd, instr.rs, instr.rt, instr_hex);
			}
			else if(!strcmp(chunk, "ROTR"))
			{
				instr.opcode = ROTR;
				decode_shift_operands(in, &instr);
				build_shift_instruction(&instr, FUNCT_SRL, 1, instr_bin);
				sprintf(instr.toString, "ROTR $%d,$%d,%d -> 0x%s\n", instr.rd, instr.rt, instr.sa, instr_hex);
			}
			else if(!strcmp(chunk, "SLL"))
			{
				instr.opcode = SLL;
				decode_shift_operands(in, &instr);
				build_shift_instruction(&instr, FUNCT_SLL, 0, instr_bin);
				sprintf(instr.toString, "SLL $%d,$%d,%d -> 0x%s\n", instr.rd, instr.rt, instr.sa, instr_hex);
			}
			else if(!strcmp(chunk, "SLT"))
			{
				instr.opcode = SLT;
				decode_r3_operands(in, &instr);
				build_r3_instruction(&instr, FUNCT_SLT, instr_bin);
				sprintf(instr.toString, "SLT $%d,$%d,$%d -> 0x%s\n", instr.rd, instr.rs, instr.rt, instr_hex);
			}
			else if(!strcmp(chunk, "SRL"))
			{
				instr.opcode = SRL;
				decode_shift_operands(in, &instr);
				build_shift_instruction(&instr, FUNCT_SRL, 0, instr_bin);
				sprintf(instr.toString, "SRL $%d,$%d,%d -> 0x%s\n", instr.rd, instr.rt, instr.sa, instr_hex);
			}
			else if(!strcmp(chunk, "SUB"))
			{
				instr.opcode = SUB;
				decode_r3_operands(in, &instr);
				build_r3_instruction(&instr, FUNCT_SUB, instr_bin);
				sprintf(instr.toString, "SUB $%d,$%d,$%d -> 0x%s\n", instr.rd, instr.rs, instr.rt, instr_hex);
			}
			else if(!strcmp(chunk, "SW"))
			{
				instr.opcode = SW;
				decode_memory_operands(in, &instr, &negative);
				build_memory_instruction(&instr, OPCODE_SW, instr_bin, negative);
				sprintf(instr.toString, "SW $%d,%d($%d) -> 0x%s\n", instr.rt, instr.offset, instr.base, instr_hex);
			}
			else if(!strcmp(chunk, "XOR"))
			{
				instr.opcode = XOR;
				decode_r3_operands(in, &instr);
				build_r3_instruction(&instr, FUNCT_XOR, instr_bin);
				sprintf(instr.toString, "XOR $%d,$%d,$%d -> 0x%s\n", instr.rd, instr.rs, instr.rt, instr_hex);
			}
		log_instruction(&instr);
		}
	}
	else
		fscanf(in, " %[^\n]\n", chunk);

	return(instr);
}