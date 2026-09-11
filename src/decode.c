#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decode.h"
#include "instruction.h"
#include "utils.h"

void build_instruction_bin(instruction_field *arr, size_t field_count, int *bin_arr, char *hex_arr)
{
	bin_zero(bin_arr);
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

void build_r3_instruction(instruction *instr, int funct_code, int *bin_arr, char *hex_arr)
{
	instruction_field fields[] = {
		FIELD(RS_START, RS_END, instr->rs),
		FIELD(RT_START, RT_END, instr->rt),
		FIELD(RD_START, RD_END, instr->rd),
		FIELD(FUNCT_START, FUNCT_END, funct_code)};
	build_instruction_bin(fields, 4, bin_arr, hex_arr);
	strcpy(instr->instr_hex, hex_arr);
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
	int instr_bin[32], negative = 0;
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
				build_r3_instruction(&instr, FUNCT_ADD, instr_bin, instr_hex);
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
				build_r3_instruction(&instr, FUNCT_AND, instr_bin, instr_hex);
				sprintf(instr.toString, "AND $%d,$%d,$%d -> 0x%s\n", instr.rd, instr.rs, instr.rt, instr_hex);
			}
			else if(!strcmp(chunk, "BEQ"))
			{
				decode_branch_r2_operands(in, &instr, &negative);
				instruction_field fields[] = {FIELD(0, 5, 4), FIELD(6, 10, instr.rt), FIELD(11, 15, instr.rs), FIELD(16, 31, instr.offset)};
				build_instruction_bin(fields, 4, instr_bin, instr_hex);
				if(negative)
				{
					instr.offset *= -1;
					bin_twos_complement(16, 31, instr_bin);
				}
				bin_arr_to_hex_arr(instr_bin, instr_hex); // TODO: to keep for now because if(negative) is after build_instruction_bin()
				strcpy(instr.instr_hex, instr_hex);
				instr.opcode = BEQ;
				sprintf(instr.toString, "BEQ $%d,$%d,%d -> 0x%s\n", instr.rs, instr.rt, instr.offset, instr_hex);
			}
			else if(!strcmp(chunk, "BGTZ"))
			{
				decode_branch_r1_operands(in, &instr, &negative);
				instruction_field fields[] = {FIELD(0, 5, 7), FIELD(6, 10, instr.rs), FIELD(16, 31, instr.offset)};
				build_instruction_bin(fields, 3, instr_bin, instr_hex);
				if(negative)
				{
					instr.offset *= -1;
					bin_twos_complement(16, 31, instr_bin);
				}
				bin_arr_to_hex_arr(instr_bin, instr_hex); // TODO: to keep for now because if(negative) is after build_instruction_bin()
				strcpy(instr.instr_hex, instr_hex);
				instr.opcode = BGTZ;
				sprintf(instr.toString, "BGTZ $%d,%d -> 0x%s\n", instr.rs, instr.offset, instr_hex);
			}
			else if(!strcmp(chunk, "BLEZ"))
			{
				decode_branch_r1_operands(in, &instr, &negative);
				instruction_field fields[] = {FIELD(0, 5, 6), FIELD(6, 10, instr.rs), FIELD(16, 31, instr.offset)};
				build_instruction_bin(fields, 3, instr_bin, instr_hex);
				if(negative)
				{
					instr.offset *= -1;
					bin_twos_complement(16, 31, instr_bin);
				}
				bin_arr_to_hex_arr(instr_bin, instr_hex); // TODO: to keep for now because if(negative) is after build_instruction_bin()
				strcpy(instr.instr_hex, instr_hex);
				instr.opcode = BLEZ;
				sprintf(instr.toString, "BLEZ $%d,%d -> 0x%s\n", instr.rs, instr.offset, instr_hex);
			}
			else if(!strcmp(chunk, "BNE"))
			{
				decode_branch_r2_operands(in, &instr, &negative);
				instruction_field fields[] = {FIELD(0, 5, 5), FIELD(6, 10, instr.rt), FIELD(11, 15, instr.rs), FIELD(16, 31, instr.offset)};
				build_instruction_bin(fields, 4, instr_bin, instr_hex);
				if(negative)
				{
					instr.offset *= -1;
					bin_twos_complement(16, 31, instr_bin);
				}
				bin_arr_to_hex_arr(instr_bin, instr_hex); // TODO: to keep for now because if(negative) is after build_instruction_bin()
				strcpy(instr.instr_hex, instr_hex);
				instr.opcode = BNE;
				sprintf(instr.toString, "BNE $%d,$%d,%d -> 0x%s\n", instr.rs, instr.rt, instr.offset, instr_hex);
			}
			else if(!strcmp(chunk, "DIV"))
			{
				decode_r2_operands(in, &instr);
				instruction_field fields[] = {FIELD(6, 10, instr.rs), FIELD(11, 15, instr.rt), FIELD(16, 31, 26)};
				build_instruction_bin(fields, 3, instr_bin, instr_hex);
				strcpy(instr.instr_hex, instr_hex);
				instr.opcode = DIV;
				sprintf(instr.toString, "DIV $%d,$%d -> 0x%s\n", instr.rs, instr.rt, instr_hex);
			}
			else if(!strcmp(chunk, "EXIT"))
				instr.exit = 1;
			else if(!strcmp(chunk, "J"))
			{
				decode_target_operands(in, &instr);
				instruction_field fields[] = {FIELD(0, 5, 2), FIELD(6, 31, instr.target)};
				build_instruction_bin(fields, 2, instr_bin, instr_hex);
				strcpy(instr.instr_hex, instr_hex);
				instr.opcode = J;
				sprintf(instr.toString, "J %d -> 0x%s\n", instr.target, instr_hex);
			}
			else if(!strcmp(chunk, "JAL"))
			{
				decode_target_operands(in, &instr);
				instruction_field fields[] = {FIELD(0, 5, 3), FIELD(6, 31, instr.target)};
				build_instruction_bin(fields, 2, instr_bin, instr_hex);
				strcpy(instr.instr_hex, instr_hex);
				instr.opcode = JAL;
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
				decode_memory_operands(in, &instr, &negative);
				instruction_field fields[] = {FIELD(0, 5, 35), FIELD(6, 10, instr.base), FIELD(11, 15, instr.rt), FIELD(16, 31, instr.offset)};
				build_instruction_bin(fields, 4, instr_bin, instr_hex);
				if(negative)
				{
					instr.offset *= -1;
					bin_twos_complement(16, 31, instr_bin);
				}
				bin_arr_to_hex_arr(instr_bin, instr_hex); // TODO: to keep for now because if(negative) is after build_instruction_bin()
				strcpy(instr.instr_hex, instr_hex);
				instr.opcode = LW;
				sprintf(instr.toString, "LW $%d,%d($%d) -> 0x%s\n", instr.rt, instr.offset, instr.base, instr_hex);
			}
			else if(!strcmp(chunk, "MFHI"))
			{
				decode_rd_operand(in, &instr);
				instruction_field fields[] = {FIELD(16, 20, instr.rd), FIELD(21, 31, 16)};
				build_instruction_bin(fields, 2, instr_bin, instr_hex);
				strcpy(instr.instr_hex, instr_hex);
				instr.opcode = MFHI;
				sprintf(instr.toString, "MFHI $%d -> 0x%s\n", instr.rd, instr_hex);
			}
			else if(!strcmp(chunk, "MFLO"))
			{
				decode_rd_operand(in, &instr);
				instruction_field fields[] = {FIELD(16, 20, instr.rd), FIELD(21, 31, 18)};
				build_instruction_bin(fields, 2, instr_bin, instr_hex);
				strcpy(instr.instr_hex, instr_hex);
				instr.opcode = MFLO;
				sprintf(instr.toString, "MFLO $%d -> 0x%s\n", instr.rd, instr_hex);
			}
			else if(!strcmp(chunk, "MULT"))
			{
				decode_r2_operands(in, &instr);
				instruction_field fields[] = {FIELD(6, 10, instr.rs), FIELD(11, 15, instr.rt), FIELD(16, 31, 24)};
				build_instruction_bin(fields, 3, instr_bin, instr_hex);
				strcpy(instr.instr_hex, instr_hex);
				instr.opcode = MULT;
				sprintf(instr.toString, "MULT $%d,$%d -> 0x%s\n", instr.rs, instr.rt, instr_hex);
			}
			else if(!strcmp(chunk, "NOP"))
			{
				bin_zero(instr_bin);
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instr_hex, instr_hex);
				instr.opcode = NOP;
				sprintf(instr.toString, "NOP -> 0x%s\n", instr_hex);
			}
			else if(!strcmp(chunk, "OR"))
			{
				instr.opcode = OR;
				decode_r3_operands(in, &instr);
				build_r3_instruction(&instr, FUNCT_OR, instr_bin, instr_hex);
				sprintf(instr.toString, "OR $%d,$%d,$%d -> 0x%s\n", instr.rd, instr.rs, instr.rt, instr_hex);
			}
			else if(!strcmp(chunk, "ROTR"))
			{
				decode_shift_operands(in, &instr);
				instruction_field fields[] = {FIELD(10, 10, 1), FIELD(11, 15, instr.rt), FIELD(16, 20, instr.rd), FIELD(21, 25, instr.sa), FIELD(26, 31, 2)};
				build_instruction_bin(fields, 5, instr_bin, instr_hex);
				strcpy(instr.instr_hex, instr_hex);
				instr.opcode = ROTR;
				sprintf(instr.toString, "ROTR $%d,$%d,%d -> 0x%s\n", instr.rd, instr.rt, instr.sa, instr_hex);
			}
			else if(!strcmp(chunk, "SLL"))
			{
				decode_shift_operands(in, &instr);
				instruction_field fields[] = {FIELD(11, 15, instr.rt), FIELD(16, 20, instr.rd), FIELD(21, 25, instr.sa)};
				build_instruction_bin(fields, 3, instr_bin, instr_hex);
				strcpy(instr.instr_hex, instr_hex);
				instr.opcode = SLL;
				sprintf(instr.toString, "SLL $%d,$%d,%d -> 0x%s\n", instr.rd, instr.rt, instr.sa, instr_hex);
			}
			else if(!strcmp(chunk, "SLT"))
			{
				instr.opcode = SLT;
				decode_r3_operands(in, &instr);
				build_r3_instruction(&instr, FUNCT_SLT, instr_bin, instr_hex);
				sprintf(instr.toString, "SLT $%d,$%d,$%d -> 0x%s\n", instr.rd, instr.rs, instr.rt, instr_hex);
			}
			else if(!strcmp(chunk, "SRL"))
			{
				decode_shift_operands(in, &instr);
				instruction_field fields[] = {FIELD(11, 15, instr.rt), FIELD(16, 20, instr.rd), FIELD(21, 25, instr.sa), FIELD(26, 31, 2)};
				build_instruction_bin(fields, 4, instr_bin, instr_hex);
				strcpy(instr.instr_hex, instr_hex);
				instr.opcode = SRL;
				sprintf(instr.toString, "SRL $%d,$%d,%d -> 0x%s\n", instr.rd, instr.rt, instr.sa, instr_hex);
			}
			else if(!strcmp(chunk, "SUB"))
			{
				instr.opcode = SUB;
				decode_r3_operands(in, &instr);
				build_r3_instruction(&instr, FUNCT_SUB, instr_bin, instr_hex);
				sprintf(instr.toString, "SUB $%d,$%d,$%d -> 0x%s\n", instr.rd, instr.rs, instr.rt, instr_hex);
			}
			else if(!strcmp(chunk, "SW"))
			{
				decode_memory_operands(in, &instr, &negative);
				instruction_field fields[] = {FIELD(0, 5, 43), FIELD(6, 10, instr.base), FIELD(11, 15, instr.rt), FIELD(16, 31, instr.offset)};
				build_instruction_bin(fields, 4, instr_bin, instr_hex);
				if(negative)
				{
					instr.offset *= -1;
					bin_twos_complement(16, 31, instr_bin);
				}
				bin_arr_to_hex_arr(instr_bin, instr_hex); // TODO: to keep for now because if(negative) is after build_instruction_bin()
				strcpy(instr.instr_hex, instr_hex);
				instr.opcode = SW;
				sprintf(instr.toString, "SW $%d,%d($%d) -> 0x%s\n", instr.rt, instr.offset, instr.base, instr_hex);
			}
			else if(!strcmp(chunk, "XOR"))
			{
				instr.opcode = XOR;
				decode_r3_operands(in, &instr);
				build_r3_instruction(&instr, FUNCT_XOR, instr_bin, instr_hex);
				sprintf(instr.toString, "XOR $%d,$%d,$%d -> 0x%s\n", instr.rd, instr.rs, instr.rt, instr_hex);
			}
		log_instruction(&instr);
		}
	}
	else
		fscanf(in, " %[^\n]\n", chunk);

	return(instr);
}