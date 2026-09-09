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

instruction decode_instruction(int mode, FILE *fichier)
{
	char bloc[50], instructionHex[9], param1[20], param2[20], param3[20];
	int instructionBin[32], signe = 0;
	instruction instr = {0};

	FILE *in = (!mode) ? stdin : fichier;

	if(fscanf(in, "%49s", bloc) != 1)
	{
		instr.exit = 1;
		return instr;
	}

	if(bloc[0] != '#')
	{
		if(bloc[0] == '.')
			printf("Pour l'instant pas de prise en charge des directives\n");
		else
		{
			if(!strcmp(bloc, "ADD"))
			{
				fscanf(in, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				instr.rd = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.rt = register_string_to_int(param3);
				instruction_field fields[] = {FIELD(6, 10, instr.rs), FIELD(11, 15, instr.rt), FIELD(16, 20, instr.rd), FIELD(21, 31, 32)};
				build_instruction_bin(fields, 4, instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = ADD;
				instr.type = 2;
				sprintf(instr.toString, "ADD $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "ADDI"))
			{
				fscanf(in, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				instr.rt = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.immediate = handle_sign(param3, &signe);
				instruction_field fields[] = {FIELD(0, 5, 8), FIELD(6, 10, instr.rs), FIELD(11, 15, instr.rt), FIELD(16, 31, instr.immediate)};
				build_instruction_bin(fields, 4, instructionBin, instructionHex);
				if(signe)
				{
					instr.immediate = instr.immediate * -1;
					bin_twos_complement(16, 31, instructionBin);
				}
				bin_arr_to_hex_arr(instructionBin, instructionHex); // TODO: to keep for now because if(signe) is after build_instruction_bin()
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = ADDI;
				instr.type = 2;
				sprintf(instr.toString, "ADDI $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "AND"))
			{
				fscanf(in, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				instr.rd = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.rt = register_string_to_int(param3);
				instruction_field fields[] = {FIELD(6, 10, instr.rs), FIELD(11, 15, instr.rt), FIELD(16, 20, instr.rd), FIELD(21, 31, 36)};
				build_instruction_bin(fields, 4, instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = AND;
				instr.type = 2;
				sprintf(instr.toString, "AND $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "BEQ"))
			{
				fscanf(in, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				instr.rt = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.offset = handle_sign(param3, &signe);
				instruction_field fields[] = {FIELD(0, 5, 4), FIELD(6, 10, instr.rt), FIELD(11, 15, instr.rs), FIELD(16, 31, instr.offset)};
				build_instruction_bin(fields, 4, instructionBin, instructionHex);
				if(signe)
				{
					instr.offset = instr.offset * -1;
					bin_twos_complement(16, 31, instructionBin);
				}
				bin_arr_to_hex_arr(instructionBin, instructionHex); // TODO: to keep for now because if(signe) is after build_instruction_bin()
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = BEQ;
				instr.type = 2;
				sprintf(instr.toString, "BEQ $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "BGTZ"))
			{
				fscanf(in, " $%[^,]  , %s ", param1, param2);
				instr.rs = register_string_to_int(param1);
				instr.offset = handle_sign(param2, &signe);
				instruction_field fields[] = {FIELD(0, 5, 7), FIELD(6, 10, instr.rs), FIELD(16, 31, instr.offset)};
				build_instruction_bin(fields, 3, instructionBin, instructionHex);
				if(signe)
				{
					instr.offset = instr.offset * -1;
					bin_twos_complement(16, 31, instructionBin);
				}
				bin_arr_to_hex_arr(instructionBin, instructionHex); // TODO: to keep for now because if(signe) is after build_instruction_bin()
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = BGTZ;
				instr.type = 2;
				sprintf(instr.toString, "BGTZ $%s,%s -> 0x%s\n", param1, param2, instructionHex);
			}
			else if(!strcmp(bloc, "BLEZ"))
			{
				fscanf(in, " $%[^,]  , %s ", param1, param2);
				instr.rs = register_string_to_int(param1);
				instr.offset = handle_sign(param2, &signe);
				instruction_field fields[] = {FIELD(0, 5, 6), FIELD(6, 10, instr.rs), FIELD(16, 31, instr.offset)};
				build_instruction_bin(fields, 3, instructionBin, instructionHex);
				if(signe)
				{
					instr.offset = instr.offset * -1;
					bin_twos_complement(16, 31, instructionBin);
				}
				bin_arr_to_hex_arr(instructionBin, instructionHex); // TODO: to keep for now because if(signe) is after build_instruction_bin()
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = BLEZ;
				instr.type = 2;
				sprintf(instr.toString, "BLEZ $%s,%s -> 0x%s\n", param1, param2, instructionHex);
			}
			else if(!strcmp(bloc, "BNE"))
			{
				fscanf(in, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				instr.rt = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.offset = handle_sign(param3, &signe);
				instruction_field fields[] = {FIELD(0, 5, 5), FIELD(6, 10, instr.rt), FIELD(11, 15, instr.rs), FIELD(16, 31, instr.offset)};
				build_instruction_bin(fields, 4, instructionBin, instructionHex);
				if(signe)
				{
					instr.offset = instr.offset * -1;
					bin_twos_complement(16, 31, instructionBin);
				}
				bin_arr_to_hex_arr(instructionBin, instructionHex); // TODO: to keep for now because if(signe) is after build_instruction_bin()
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = BNE;
				instr.type = 2;
				sprintf(instr.toString, "BNE $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "DIV"))
			{
				fscanf(in, " $%[^,] , $%s ", param1, param2);
				instr.rs = register_string_to_int(param1);
				instr.rt = register_string_to_int(param2);
				instruction_field fields[] = {FIELD(6, 10, instr.rs), FIELD(11, 15, instr.rt), FIELD(16, 31, 26)};
				build_instruction_bin(fields, 3, instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = DIV;
				instr.type = 2;
				sprintf(instr.toString, "DIV $%s,$%s -> 0x%s\n", param1, param2, instructionHex);
			}
			else if(!strcmp(bloc, "EXIT"))
				instr.exit = 1;
			else if(!strcmp(bloc, "J"))
			{
				fscanf(in, " %s ", param1);
				instr.target = atoi(param1);
				instruction_field fields[] = {FIELD(0, 5, 2), FIELD(6, 31, instr.target)};
				build_instruction_bin(fields, 2, instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = J;
				instr.type = 2;
				sprintf(instr.toString, "J %s -> 0x%s\n", param1, instructionHex);
			}
			else if(!strcmp(bloc, "JAL"))
			{
				fscanf(in, " %s ", param1);
				instr.target = atoi(param1);
				instruction_field fields[] = {FIELD(0, 5, 3), FIELD(6, 31, instr.target)};
				build_instruction_bin(fields, 2, instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = JAL;
				instr.type = 2;
				sprintf(instr.toString, "JAL %s -> 0x%s\n", param1, instructionHex);
			}
			else if(!strcmp(bloc, "JR"))
			{
				fscanf(in, " $%s ", param1);
				instr.rs = register_string_to_int(param1);
				instruction_field fields[] = {FIELD(6, 10, instr.rs), FIELD(11, 31, 8)};
				build_instruction_bin(fields, 2, instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = JR;
				instr.type = 2;
				sprintf(instr.toString, "JR $%s -> 0x%s\n", param1, instructionHex);
			}
			else if(!strcmp(bloc, "LUI"))
			{
				fscanf(in, " $%[^,] , %s ", param1, param2);
				instr.rt = register_string_to_int(param1);
				instr.immediate = atoi(param2);
				instruction_field fields[] = {FIELD(0, 5, 15), FIELD(11, 15, instr.rt), FIELD(16, 31, instr.immediate)};
				build_instruction_bin(fields, 3, instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = LUI;
				instr.type = 2;
				sprintf(instr.toString, "LUI $%s,%s -> 0x%s\n", param1, param2, instructionHex);
			}
			else if(!strcmp(bloc, "LW"))
			{
				fscanf(in, " $%[^,] , %[^(] ($%[^)]) ", param1, param2, param3);
				instr.rt = register_string_to_int(param1);
				instr.offset = handle_sign(param2, &signe);
				instr.base = register_string_to_int(param3);
				instruction_field fields[] = {FIELD(0, 5, 35), FIELD(6, 10, instr.base), FIELD(11, 15, instr.rt), FIELD(16, 31, instr.offset)};
				build_instruction_bin(fields, 4, instructionBin, instructionHex);
				if(signe)
				{
					instr.offset = instr.offset * -1;
					bin_twos_complement(16, 31, instructionBin);
				}
				bin_arr_to_hex_arr(instructionBin, instructionHex); // TODO: to keep for now because if(signe) is after build_instruction_bin()
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = LW;
				instr.type = 2;
				sprintf(instr.toString, "LW $%s,%s($%s) -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "MFHI"))
			{
				fscanf(in, " $%s ", param1);
				instr.rd = register_string_to_int(param1);
				instruction_field fields[] = {FIELD(16, 20, instr.rd), FIELD(21, 31, 16)};
				build_instruction_bin(fields, 2, instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = MFHI;
				instr.type = 2;
				sprintf(instr.toString, "MFHI $%s -> 0x%s\n", param1, instructionHex);
			}
			else if(!strcmp(bloc, "MFLO"))
			{
				fscanf(in, " $%s ", param1);
				instr.rd = register_string_to_int(param1);
				instruction_field fields[] = {FIELD(16, 20, instr.rd), FIELD(21, 31, 18)};
				build_instruction_bin(fields, 2, instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = MFLO;
				instr.type = 2;
				sprintf(instr.toString, "MFLO $%s -> 0x%s\n", param1, instructionHex);
			}
			else if(!strcmp(bloc, "MULT"))
			{
				fscanf(in, " $%[^,] , $%s ", param1, param2);
				instr.rs = register_string_to_int(param1);
				instr.rt = register_string_to_int(param2);
				instruction_field fields[] = {FIELD(6, 10, instr.rs), FIELD(11, 15, instr.rt), FIELD(16, 31, 24)};
				build_instruction_bin(fields, 3, instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = MULT;
				instr.type = 2;
				sprintf(instr.toString, "MULT $%s,$%s -> 0x%s\n", param1, param2, instructionHex);
			}
			else if(!strcmp(bloc, "NOP"))
			{
				bin_zero(instructionBin);
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = NOP;
				instr.type = 2;
				sprintf(instr.toString, "NOP -> 0x%s\n", instructionHex);
			}
			else if(!strcmp(bloc, "OR"))
			{
				fscanf(in, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				instr.rd = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.rt = register_string_to_int(param3);
				instruction_field fields[] = {FIELD(6, 10, instr.rs), FIELD(11, 15, instr.rt), FIELD(16, 20, instr.rd), FIELD(21, 31, 37)};
				build_instruction_bin(fields, 4, instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = OR;
				instr.type = 2;
				sprintf(instr.toString, "OR $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "ROTR"))
			{
				fscanf(in, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				instr.rd = register_string_to_int(param1);
				instr.rt = register_string_to_int(param2);
				instr.sa = atoi(param3);
				printf("dbg:%d\n", instr.sa);
				instruction_field fields[] = {FIELD(10, 10, 1), FIELD(11, 15, instr.rt), FIELD(16, 20, instr.rd), FIELD(21, 25, instr.sa), FIELD(26, 31, 2)};
				build_instruction_bin(fields, 5, instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = ROTR;
				instr.type = 2;
				sprintf(instr.toString, "ROTR $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "SLL"))
			{
				fscanf(in, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				instr.rd = register_string_to_int(param1);
				instr.rt = register_string_to_int(param2);
				instr.sa = atoi(param3);
				instruction_field fields[] = {FIELD(11, 15, instr.rt), FIELD(16, 20, instr.rd), FIELD(21, 25, instr.sa)};
				build_instruction_bin(fields, 3, instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = SLL;
				instr.type = 2;
				sprintf(instr.toString, "SLL $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "SLT"))
			{
				fscanf(in, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				instr.rd = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.rt = register_string_to_int(param3);
				instruction_field fields[] = {FIELD(6, 10, instr.rs), FIELD(11, 15, instr.rt), FIELD(16, 20, instr.rd), FIELD(21, 31, 42)};
				build_instruction_bin(fields, 4, instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = SLT;
				instr.type = 2;
				sprintf(instr.toString, "SLT $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "SRL"))
			{
				fscanf(in, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				instr.rd = register_string_to_int(param1);
				instr.rt = register_string_to_int(param2);
				instr.sa = atoi(param3);
				instruction_field fields[] = {FIELD(11, 15, instr.rt), FIELD(16, 20, instr.rd), FIELD(21, 25, instr.sa), FIELD(26, 31, 2)};
				build_instruction_bin(fields, 4, instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = SRL;
				instr.type = 2;
				sprintf(instr.toString, "SRL $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "SUB"))
			{
				fscanf(in, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				instr.rd = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.rt = register_string_to_int(param3);
				instruction_field fields[] = {FIELD(6, 10, instr.rs), FIELD(11, 15, instr.rt), FIELD(16, 20, instr.rd), FIELD(21, 31, 34)};
				build_instruction_bin(fields, 4, instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = SUB;
				instr.type = 2;
				sprintf(instr.toString, "SUB $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "SW"))
			{
				fscanf(in, " $%[^,] , %[^(] ($%[^)]) ", param1, param2, param3);
				instr.rt = register_string_to_int(param1);
				instr.offset = handle_sign(param2, &signe);
				instr.base = register_string_to_int(param3);
				instruction_field fields[] = {FIELD(0, 5, 43), FIELD(6, 10, instr.base), FIELD(11, 15, instr.rt), FIELD(16, 31, instr.offset)};
				build_instruction_bin(fields, 4, instructionBin, instructionHex);
				if(signe)
				{
					instr.offset = instr.offset * -1;
					bin_twos_complement(16, 31, instructionBin);
				}
				bin_arr_to_hex_arr(instructionBin, instructionHex); // TODO: to keep for now because if(signe) is after build_instruction_bin()
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = SW;
				instr.type = 2;
				sprintf(instr.toString, "SW $%s,%s($%s) -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "XOR"))
			{
				fscanf(in, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				instr.rd = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.rt = register_string_to_int(param3);
				instruction_field fields[] = {FIELD(6, 10, instr.rs), FIELD(11, 15, instr.rt), FIELD(16, 20, instr.rd), FIELD(21, 31, 38)};
				build_instruction_bin(fields, 4, instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = XOR;
				instr.type = 2;
				sprintf(instr.toString, "XOR $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
		log_instruction(&instr);
		}
	}
	else
	{
		instr.type = -1;
		fscanf(in, " %[^\n]\n", bloc);
	}

	return(instr);
}