#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decode.h"
#include "instruction.h"
#include "utils.h"

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
			printf("Pour l'instant pas de prise en charge des directives\n");
		else
		{
			if(!strcmp(chunk, "ADD"))
			{
				fscanf(in, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				bin_zero(instr_bin);
				instr.rd = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.rt = register_string_to_int(param3);
				long_to_bin_arr(6, 10, instr.rs, instr_bin);
				long_to_bin_arr(11, 15, instr.rt, instr_bin);
				long_to_bin_arr(16, 20, instr.rd, instr_bin);
				long_to_bin_arr(21, 31, 32, instr_bin);
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = ADD;
				instr.type = 2;
				sprintf(instr.toString, "ADD $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instr_hex);
			}
			else if(!strcmp(chunk, "ADDI"))
			{
				fscanf(in, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				if(param3[0] == '-')
				{
					negative = 1;
					remove_sign(param3);
				}
				bin_zero(instr_bin);
				instr.rt = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.immediate = atoi(param3);
				long_to_bin_arr(0, 5, 8, instr_bin);
				long_to_bin_arr(6, 10, instr.rs, instr_bin);
				long_to_bin_arr(11, 15, instr.rt, instr_bin);
				long_to_bin_arr(16, 31, instr.immediate, instr_bin);
				if(negative)
				{
					instr.immediate = instr.immediate * -1;
					bin_twos_complement(16, 31, instr_bin);
					negative = 0;
				}
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = ADDI;
				instr.type = 2;
				sprintf(instr.toString, "ADDI $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instr_hex);
			}
			else if(!strcmp(chunk, "AND"))
			{
				fscanf(in, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				bin_zero(instr_bin);
				instr.rd = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.rt = register_string_to_int(param3);
				long_to_bin_arr(16, 20, instr.rd, instr_bin);
				long_to_bin_arr(6, 10, instr.rs, instr_bin);
				long_to_bin_arr(11, 15, instr.rt, instr_bin);
				long_to_bin_arr(21, 31, 36, instr_bin);
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = AND;
				instr.type = 2;
				sprintf(instr.toString, "AND $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instr_hex);
			}
			else if(!strcmp(chunk, "BEQ"))
			{
				fscanf(in, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				if(param3[0] == '-')
				{
					negative = 1;
					remove_sign(param3);
				}
				bin_zero(instr_bin);
				instr.rt = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.offset = atoi(param3);
				long_to_bin_arr(6, 10, instr.rt, instr_bin);
				long_to_bin_arr(11, 15, instr.rs, instr_bin);
				long_to_bin_arr(16, 31, instr.offset, instr_bin);
				long_to_bin_arr(0, 5, 4, instr_bin);
				if(negative)
				{
					instr.immediate = instr.immediate * -1;
					bin_twos_complement(16, 31, instr_bin);
					negative = 0;
				}
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = BEQ;
				instr.type = 2;
				sprintf(instr.toString, "BEQ $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instr_hex);
			}
			else if(!strcmp(chunk, "BGTZ"))
			{
				fscanf(in, " $%[^,]  , %s ", param1, param2);
				if(param2[0] == '-')
				{
					negative = 1;
					remove_sign(param2);
				}
				bin_zero(instr_bin);
				instr.rs = register_string_to_int(param1);
				instr.offset = atoi(param2);
				long_to_bin_arr(6, 10, instr.rs, instr_bin);
				long_to_bin_arr(16, 31, instr.offset, instr_bin);
				long_to_bin_arr(0, 5, 7, instr_bin);
				if(negative)
				{
					instr.offset = instr.offset * -1;
					bin_twos_complement(16, 31, instr_bin);
					negative = 0;
				}
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = BGTZ;
				instr.type = 2;
				sprintf(instr.toString, "BGTZ $%s,%s -> 0x%s\n", param1, param2, instr_hex);
			}
			else if(!strcmp(chunk, "BLEZ"))
			{
				fscanf(in, " $%[^,]  , %s ", param1, param2);
				if(param2[0] == '-')
				{
					negative = 1;
					remove_sign(param2);
				}
				bin_zero(instr_bin);
				instr.rs = register_string_to_int(param1);
				instr.offset = atoi(param2);
				long_to_bin_arr(6, 10, instr.rs, instr_bin);
				long_to_bin_arr(16, 31, instr.offset, instr_bin);
				long_to_bin_arr(0, 5, 6, instr_bin);
				if(negative)
				{
					instr.offset = instr.offset * -1;
					bin_twos_complement(16, 31, instr_bin);
					negative = 0;
				}
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = BLEZ;
				instr.type = 2;
				sprintf(instr.toString, "BLEZ $%s,%s -> 0x%s\n", param1, param2, instr_hex);
			}
			else if(!strcmp(chunk, "BNE"))
			{
				fscanf(in, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				if(param3[0] == '-')
				{
					negative = 1;
					remove_sign(param3);
				}
				bin_zero(instr_bin);
				instr.rt = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.offset = atoi(param3);
				long_to_bin_arr(6, 10, instr.rt, instr_bin);
				long_to_bin_arr(11, 15, instr.rs, instr_bin);
				long_to_bin_arr(16, 31, instr.offset, instr_bin);
				long_to_bin_arr(0, 5, 5, instr_bin);
				if(negative)
				{
					instr.offset = instr.offset * -1;
					bin_twos_complement(16, 31, instr_bin);
					negative = 0;
				}
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = BNE;
				instr.type = 2;
				sprintf(instr.toString, "BNE $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instr_hex);
			}
			else if(!strcmp(chunk, "DIV"))
			{
				fscanf(in, " $%[^,] , $%s ", param1, param2);
				bin_zero(instr_bin);
				instr.rs = register_string_to_int(param1);
				instr.rt = register_string_to_int(param2);
				long_to_bin_arr(6, 10, instr.rs, instr_bin);
				long_to_bin_arr(11, 15, instr.rt, instr_bin);
				long_to_bin_arr(16, 31, 26, instr_bin);
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = DIV;
				instr.type = 2;
				sprintf(instr.toString, "DIV $%s,$%s -> 0x%s\n", param1, param2, instr_hex);
			}
			else if(!strcmp(chunk, "EXIT"))
				instr.exit = 1;
			else if(!strcmp(chunk, "J"))
			{
				fscanf(in, " %s ", param1);
				bin_zero(instr_bin);
				instr.target = register_string_to_int(param1);
				long_to_bin_arr(6, 31, instr.target, instr_bin);
				long_to_bin_arr(0, 5, 2, instr_bin);
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = J;
				instr.type = 2;
				sprintf(instr.toString, "J %s -> 0x%s\n", param1, instr_hex);
			}
			else if(!strcmp(chunk, "JAL"))
			{
				fscanf(in, " %s ", param1);
				bin_zero(instr_bin);
				instr.target = register_string_to_int(param1);
				long_to_bin_arr(6, 31, instr.target, instr_bin);
				long_to_bin_arr(0, 5, 3, instr_bin);
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = JAL;
				instr.type = 2;
				sprintf(instr.toString, "JAL %s -> 0x%s\n", param1, instr_hex);
			}
			else if(!strcmp(chunk, "JR"))
			{
				fscanf(in, " $%s ", param1);
				bin_zero(instr_bin);
				instr.rs = register_string_to_int(param1);
				long_to_bin_arr(6, 10, instr.rs, instr_bin);
				long_to_bin_arr(11, 31, 8, instr_bin);
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = JR;
				instr.type = 2;
				sprintf(instr.toString, "JR $%s -> 0x%s\n", param1, instr_hex);
			}
			else if(!strcmp(chunk, "LUI"))
			{
				fscanf(in, " $%[^,] , %s ", param1, param2);
				bin_zero(instr_bin);
				instr.rt = register_string_to_int(param1);
				instr.immediate = register_string_to_int(param2);
				long_to_bin_arr(11, 15, instr.rt, instr_bin);
				long_to_bin_arr(16, 31, instr.immediate, instr_bin);
				long_to_bin_arr(0, 5, 15, instr_bin);
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = LUI;
				instr.type = 2;
				sprintf(instr.toString, "LUI $%s,%s -> 0x%s\n", param1, param2, instr_hex);
			}
			else if(!strcmp(chunk, "LW"))
			{
				fscanf(in, " $%[^,] , %[^(] ($%[^)]) ", param1, param2, param3);
				if(param2[0] == '-')
				{
					negative = 1;
					remove_sign(param2);
				}
				bin_zero(instr_bin);
				instr.rt = register_string_to_int(param1);
				instr.offset = register_string_to_int(param2);
				instr.base = register_string_to_int(param3);
				long_to_bin_arr(11, 15, instr.rt, instr_bin);
				long_to_bin_arr(16, 31, instr.offset, instr_bin);
				long_to_bin_arr(6, 10, instr.base, instr_bin);
				long_to_bin_arr(0, 5, 35, instr_bin);
				if(negative)
				{
					instr.offset = instr.offset * -1;
					bin_twos_complement(16, 31, instr_bin);
					negative = 0;
				}
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = LW;
				instr.type = 2;
				sprintf(instr.toString, "LW $%s,%s($%s) -> 0x%s\n", param1, param2, param3, instr_hex);
			}
			else if(!strcmp(chunk, "MFHI"))
			{
				fscanf(in, " $%s ", param1);
				bin_zero(instr_bin);
				instr.rd = register_string_to_int(param1);
				long_to_bin_arr(16, 20, instr.rd, instr_bin);
				long_to_bin_arr(21, 31, 16, instr_bin);
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = MFHI;
				instr.type = 2;
				sprintf(instr.toString, "MFHI $%s -> 0x%s\n", param1, instr_hex);
			}
			else if(!strcmp(chunk, "MFLO"))
			{
				fscanf(in, " $%s ", param1);
				bin_zero(instr_bin);
				instr.rd = register_string_to_int(param1);
				long_to_bin_arr(16, 20, instr.rd, instr_bin);
				long_to_bin_arr(21, 31, 18, instr_bin);
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = MFLO;
				instr.type = 2;
				sprintf(instr.toString, "MFLO $%s -> 0x%s\n", param1, instr_hex);
			}
			else if(!strcmp(chunk, "MULT"))
			{
				fscanf(in, " $%[^,] , $%s ", param1, param2);
				bin_zero(instr_bin);
				instr.rs = register_string_to_int(param1);
				instr.rt = register_string_to_int(param2);
				long_to_bin_arr(6, 10, instr.rs, instr_bin);
				long_to_bin_arr(11, 15, instr.rt, instr_bin);
				long_to_bin_arr(16, 31, 24, instr_bin);
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = MULT;
				instr.type = 2;
				sprintf(instr.toString, "MULT $%s,$%s -> 0x%s\n", param1, param2, instr_hex);
			}
			else if(!strcmp(chunk, "NOP"))
			{
				bin_zero(instr_bin);
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = NOP;
				instr.type = 2;
				sprintf(instr.toString, "NOP -> 0x%s\n", instr_hex);
			}
			else if(!strcmp(chunk, "OR"))
			{
				fscanf(in, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				bin_zero(instr_bin);
				instr.rd = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.rt = register_string_to_int(param3);
				long_to_bin_arr(16, 20, instr.rd, instr_bin);
				long_to_bin_arr(6, 10, instr.rs, instr_bin);
				long_to_bin_arr(11, 15, instr.rt, instr_bin);
				long_to_bin_arr(21, 31, 37, instr_bin);
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = OR;
				instr.type = 2;
				sprintf(instr.toString, "OR $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instr_hex);
			}
			else if(!strcmp(chunk, "ROTR"))
			{
				fscanf(in, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				bin_zero(instr_bin);
				instr.rd = register_string_to_int(param1);
				instr.rt = register_string_to_int(param2);
				instr.sa = register_string_to_int(param3);
				long_to_bin_arr(16, 20, instr.rd, instr_bin);
				long_to_bin_arr(11, 15, instr.rt, instr_bin);
				long_to_bin_arr(21, 25, instr.sa, instr_bin);
				long_to_bin_arr(10, 10, 1, instr_bin);
				long_to_bin_arr(26, 31, 2, instr_bin);
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = ROTR;
				instr.type = 2;
				sprintf(instr.toString, "ROTR $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instr_hex);
			}
			else if(!strcmp(chunk, "SLL"))
			{
				fscanf(in, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				bin_zero(instr_bin);
				instr.rd = register_string_to_int(param1);
				instr.rt = register_string_to_int(param2);
				instr.sa = register_string_to_int(param3);
				long_to_bin_arr(16, 20, instr.rd, instr_bin);
				long_to_bin_arr(11, 15, instr.rt, instr_bin);
				long_to_bin_arr(21, 25, instr.sa, instr_bin);
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = SLL;
				instr.type = 2;
				sprintf(instr.toString, "SLL $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instr_hex);
			}
			else if(!strcmp(chunk, "SLT"))
			{
				fscanf(in, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				bin_zero(instr_bin);
				instr.rd = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.rt = register_string_to_int(param3);
				long_to_bin_arr(16, 20, instr.rd, instr_bin);
				long_to_bin_arr(6, 10, instr.rs, instr_bin);
				long_to_bin_arr(11, 15, instr.rt, instr_bin);
				long_to_bin_arr(21, 31, 42, instr_bin);
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = SLT;
				instr.type = 2;
				sprintf(instr.toString, "SLT $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instr_hex);
			}
			else if(!strcmp(chunk, "SRL"))
			{
				fscanf(in, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				bin_zero(instr_bin);
				instr.rd = register_string_to_int(param1);
				instr.rt = register_string_to_int(param2);
				instr.sa = register_string_to_int(param3);
				long_to_bin_arr(16, 20, instr.rd, instr_bin);
				long_to_bin_arr(11, 15, instr.rt, instr_bin);
				long_to_bin_arr(21, 25, instr.sa, instr_bin);
				long_to_bin_arr(26, 31, 2, instr_bin);
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = SRL;
				instr.type = 2;
				sprintf(instr.toString, "SRL $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instr_hex);
			}
			else if(!strcmp(chunk, "SUB"))
			{
				fscanf(in, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				bin_zero(instr_bin);
				instr.rd = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.rt = register_string_to_int(param3);
				long_to_bin_arr(16, 20, instr.rd, instr_bin);
				long_to_bin_arr(6, 10, instr.rs, instr_bin);
				long_to_bin_arr(11, 15, instr.rt, instr_bin);
				long_to_bin_arr(21, 31, 34, instr_bin);
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = SUB;
				instr.type = 2;
				sprintf(instr.toString, "SUB $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instr_hex);
			}
			else if(!strcmp(chunk, "SW"))
			{
				fscanf(in, " $%[^,] , %[^(] ($%[^)]) ", param1, param2, param3);
				if(param2[0] == '-')
				{
					negative = 1;
					remove_sign(param2);
				}
				bin_zero(instr_bin);
				instr.rt = register_string_to_int(param1);
				instr.offset = register_string_to_int(param2);
				instr.base = register_string_to_int(param3);
				long_to_bin_arr(11, 15, instr.rt, instr_bin);
				long_to_bin_arr(16, 31, instr.offset, instr_bin);
				long_to_bin_arr(6, 10, instr.base, instr_bin);
				long_to_bin_arr(0, 5, 43, instr_bin);
				if(negative)
				{
					instr.offset = instr.offset * -1;
					bin_twos_complement(16, 31, instr_bin);
					negative = 0;
				}
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = SW;
				instr.type = 2;
				sprintf(instr.toString, "SW $%s,%s($%s) -> 0x%s\n", param1, param2, param3, instr_hex);
			}
			else if(!strcmp(chunk, "XOR"))
			{
				fscanf(in, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				bin_zero(instr_bin);
				instr.rd = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.rt = register_string_to_int(param3);
				long_to_bin_arr(16, 20, instr.rd, instr_bin);
				long_to_bin_arr(6, 10, instr.rs, instr_bin);
				long_to_bin_arr(11, 15, instr.rt, instr_bin);
				long_to_bin_arr(21, 31, 38, instr_bin);
				bin_arr_to_hex_arr(instr_bin, instr_hex);
				strcpy(instr.instrHex, instr_hex);
				instr.opcode = XOR;
				instr.type = 2;
				sprintf(instr.toString, "XOR $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instr_hex);
			}
		log_instruction(&instr);
		}
	}
	else
	{
		instr.type = -1;
		fscanf(in, " %[^\n]\n", chunk);
	}

	return(instr);
}