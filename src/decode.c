#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decode.h"
#include "instruction.h"
#include "utils.h"

instruction decode_instruction(int mode, FILE *fichier)
{
	char bloc[50];
	char param1[20], param2[20], param3[20];
	int instructionBin[32];
	char instructionHex[9];
	int signe = 0;
	instruction instr = {0};
	if(mode == 0){
		fscanf(stdin, "%s", bloc);
	}
	else if(mode == 1){
		fscanf(fichier, "%s", bloc);
	}
	if(bloc[0] != '#'){
		if(bloc[0] == '.'){
			printf("Pour l'instant pas de prise en charge des directives\n");
		}
		else{
			if(!strcmp(bloc, "ADD")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , $%s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				}
				bin_zero(instructionBin);
				instr.rd = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.rt = register_string_to_int(param3);
				long_to_bin_arr(16, 20, instr.rd, instructionBin);
				long_to_bin_arr(6, 10, instr.rs, instructionBin);
				long_to_bin_arr(11, 15, instr.rt, instructionBin);
				long_to_bin_arr(21, 31, 32, instructionBin);
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = ADD;
				instr.type = 2;
				sprintf(instr.toString, "ADD $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instructionHex);
				printf("ADD $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "ADDI")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , %s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				}
				if(param3[0] == '-'){
					signe = 1;
					remove_sign(param3);
				}
				bin_zero(instructionBin);
				instr.rt = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.immediate = atoi(param3);
				long_to_bin_arr(11, 15, instr.rt, instructionBin);
				long_to_bin_arr(6, 10, instr.rs, instructionBin);
				long_to_bin_arr(16, 31, instr.immediate, instructionBin);
				long_to_bin_arr(0, 5, 8, instructionBin);
				if(signe){
					instr.immediate = instr.immediate * -1;
					bin_twos_complement(16, 31, instructionBin);
					signe = 0;
				}
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = ADDI;
				instr.type = 2;
				sprintf(instr.toString, "ADDI $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
				printf("ADDI $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "AND")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , $%s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				}
				bin_zero(instructionBin);
				instr.rd = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.rt = register_string_to_int(param3);
				long_to_bin_arr(16, 20, instr.rd, instructionBin);
				long_to_bin_arr(6, 10, instr.rs, instructionBin);
				long_to_bin_arr(11, 15, instr.rt, instructionBin);
				long_to_bin_arr(21, 31, 36, instructionBin);
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = AND;
				instr.type = 2;
				sprintf(instr.toString, "AND $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instructionHex);
				printf("AND $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "BEQ")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , %s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				}
				if(param3[0] == '-'){
					signe = 1;
					remove_sign(param3);
				}
				bin_zero(instructionBin);
				instr.rt = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.offset = atoi(param3);
				long_to_bin_arr(6, 10, instr.rt, instructionBin);
				long_to_bin_arr(11, 15, instr.rs, instructionBin);
				long_to_bin_arr(16, 31, instr.offset, instructionBin);
				long_to_bin_arr(0, 5, 4, instructionBin);
				if(signe){
					instr.immediate = instr.immediate * -1;
					bin_twos_complement(16, 31, instructionBin);
					signe = 0;
				}
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = BEQ;
				instr.type = 2;
				sprintf(instr.toString, "BEQ $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
				printf("BEQ $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "BGTZ")){
				if(mode == 0){
					fscanf(stdin, " $%[^,]  , %s", param1, param2);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,]  , %s ", param1, param2);
				}
				if(param2[0] == '-'){
					signe = 1;
					remove_sign(param2);
				}
				bin_zero(instructionBin);
				instr.rs = register_string_to_int(param1);
				instr.offset = atoi(param2);
				long_to_bin_arr(6, 10, instr.rs, instructionBin);
				long_to_bin_arr(16, 31, instr.offset, instructionBin);
				long_to_bin_arr(0, 5, 7, instructionBin);
				if(signe){
					instr.offset = instr.offset * -1;
					bin_twos_complement(16, 31, instructionBin);
					signe = 0;
				}
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = BGTZ;
				instr.type = 2;
				sprintf(instr.toString, "BGTZ $%s,%s -> 0x%s\n", param1, param2, instructionHex);
				printf("BGTZ $%s,%s -> 0x%s\n", param1, param2, instructionHex);
			}
			else if(!strcmp(bloc, "BLEZ")){
				if(mode == 0){
					fscanf(stdin, " $%[^,]  , %s", param1, param2);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,]  , %s ", param1, param2);
				}
				if(param2[0] == '-'){
					signe = 1;
					remove_sign(param2);
				}
				bin_zero(instructionBin);
				instr.rs = register_string_to_int(param1);
				instr.offset = atoi(param2);
				long_to_bin_arr(6, 10, instr.rs, instructionBin);
				long_to_bin_arr(16, 31, instr.offset, instructionBin);
				long_to_bin_arr(0, 5, 6, instructionBin);
				if(signe){
					instr.offset = instr.offset * -1;
					bin_twos_complement(16, 31, instructionBin);
					signe = 0;
				}
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = BLEZ;
				instr.type = 2;
				sprintf(instr.toString, "BLEZ $%s,%s -> 0x%s\n", param1, param2, instructionHex);
				printf("BLEZ $%s,%s -> 0x%s\n", param1, param2, instructionHex);
			}
			else if(!strcmp(bloc, "BNE")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , %s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				}
				if(param3[0] == '-'){
					signe = 1;
					remove_sign(param3);
				}
				bin_zero(instructionBin);
				instr.rt = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.offset = atoi(param3);
				long_to_bin_arr(6, 10, instr.rt, instructionBin);
				long_to_bin_arr(11, 15, instr.rs, instructionBin);
				long_to_bin_arr(16, 31, instr.offset, instructionBin);
				long_to_bin_arr(0, 5, 5, instructionBin);
				if(signe){
					instr.offset = instr.offset * -1;
					bin_twos_complement(16, 31, instructionBin);
					signe = 0;
				}
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = BNE;
				instr.type = 2;
				sprintf(instr.toString, "BNE $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
				printf("BNE $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "DIV")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%s", param1, param2);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%s ", param1, param2);
				}
				bin_zero(instructionBin);
				instr.rs = register_string_to_int(param1);
				instr.rt = register_string_to_int(param2);
				long_to_bin_arr(6, 10, instr.rs, instructionBin);
				long_to_bin_arr(11, 15, instr.rt, instructionBin);
				long_to_bin_arr(16, 31, 26, instructionBin);
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = DIV;
				instr.type = 2;
				sprintf(instr.toString, "DIV $%s,$%s -> 0x%s\n", param1, param2, instructionHex);
				printf("DIV $%s,$%s -> 0x%s\n", param1, param2, instructionHex);
			}
			else if(!strcmp(bloc, "EXIT")){
				instr.exit = 1;
			}
			else if(!strcmp(bloc, "J")){
				if(mode == 0){
					fscanf(stdin, " %s", param1);
				}
				else if(mode == 1){
					fscanf(fichier, " %s ", param1);
				}
				bin_zero(instructionBin);
				instr.target = register_string_to_int(param1);
				long_to_bin_arr(6, 31, instr.target, instructionBin);
				long_to_bin_arr(0, 5, 2, instructionBin);
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = J;
				instr.type = 2;
				sprintf(instr.toString, "J %s -> 0x%s\n", param1, instructionHex);
				printf("J %s -> 0x%s\n", param1, instructionHex);
			}
			else if(!strcmp(bloc, "JAL")){
				if(mode == 0){
					fscanf(stdin, " %s", param1);
				}
				else if(mode == 1){
					fscanf(fichier, " %s ", param1);
				}
				bin_zero(instructionBin);
				instr.target = register_string_to_int(param1);
				long_to_bin_arr(6, 31, instr.target, instructionBin);
				long_to_bin_arr(0, 5, 3, instructionBin);
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = JAL;
				instr.type = 2;
				sprintf(instr.toString, "JAL %s -> 0x%s\n", param1, instructionHex);
				printf("JAL %s -> 0x%s\n", param1, instructionHex);
			}
			else if(!strcmp(bloc, "JR")){
				if(mode == 0){
					fscanf(stdin, " $%s", param1);
				}
				else if(mode == 1){
					fscanf(fichier, " $%s ", param1);
				}
				bin_zero(instructionBin);
				instr.rs = register_string_to_int(param1);
				long_to_bin_arr(6, 10, instr.rs, instructionBin);
				long_to_bin_arr(11, 31, 8, instructionBin);
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = JR;
				instr.type = 2;
				sprintf(instr.toString, "JR $%s -> 0x%s\n", param1, instructionHex);
				printf("JR $%s -> 0x%s\n", param1, instructionHex);
			}
			else if(!strcmp(bloc, "LUI")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , %s", param1, param2);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , %s ", param1, param2);
				}
				bin_zero(instructionBin);
				instr.rt = register_string_to_int(param1);
				instr.immediate = register_string_to_int(param2);
				long_to_bin_arr(11, 15, instr.rt, instructionBin);
				long_to_bin_arr(16, 31, instr.immediate, instructionBin);
				long_to_bin_arr(0, 5, 15, instructionBin);
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = LUI;
				instr.type = 2;
				sprintf(instr.toString, "LUI $%s,%s -> 0x%s\n", param1, param2, instructionHex);
				printf("LUI $%s,%s -> 0x%s\n", param1, param2, instructionHex);
			}
			else if(!strcmp(bloc, "LW")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , %[^(] ($%[^)])", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , %[^(] ($%[^)]) ", param1, param2, param3);
				}
				if(param2[0] == '-'){
					signe = 1;
					remove_sign(param2);
				}
				bin_zero(instructionBin);
				instr.rt = register_string_to_int(param1);
				instr.offset = register_string_to_int(param2);
				instr.base = register_string_to_int(param3);
				long_to_bin_arr(11, 15, instr.rt, instructionBin);
				long_to_bin_arr(16, 31, instr.offset, instructionBin);
				long_to_bin_arr(6, 10, instr.base, instructionBin);
				long_to_bin_arr(0, 5, 35, instructionBin);
				if(signe){
					instr.offset = instr.offset * -1;
					bin_twos_complement(16, 31, instructionBin);
					signe = 0;
				}
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = LW;
				instr.type = 2;
				sprintf(instr.toString, "LW $%s,%s($%s) -> 0x%s\n", param1, param2, param3, instructionHex);
				printf("LW $%s,%s($%s) -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "MFHI")){
				if(mode == 0){
					fscanf(stdin, " $%s", param1);
				}
				else if(mode == 1){
					fscanf(fichier, " $%s ", param1);
				}
				bin_zero(instructionBin);
				instr.rd = register_string_to_int(param1);
				long_to_bin_arr(16, 20, instr.rd, instructionBin);
				long_to_bin_arr(21, 31, 16, instructionBin);
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = MFHI;
				instr.type = 2;
				sprintf(instr.toString, "MFHI $%s -> 0x%s\n", param1, instructionHex);
				printf("MFHI $%s -> 0x%s\n", param1, instructionHex);
			}
			else if(!strcmp(bloc, "MFLO")){
				if(mode == 0){
					fscanf(stdin, " $%s", param1);
				}
				else if(mode == 1){
					fscanf(fichier, " $%s ", param1);
				}
				bin_zero(instructionBin);
				instr.rd = register_string_to_int(param1);
				long_to_bin_arr(16, 20, instr.rd, instructionBin);
				long_to_bin_arr(21, 31, 18, instructionBin);
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = MFLO;
				instr.type = 2;
				sprintf(instr.toString, "MFLO $%s -> 0x%s\n", param1, instructionHex);
				printf("MFLO $%s -> 0x%s\n", param1, instructionHex);
			}
			else if(!strcmp(bloc, "MULT")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%s", param1, param2);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%s ", param1, param2);
				}
				bin_zero(instructionBin);
				instr.rs = register_string_to_int(param1);
				instr.rt = register_string_to_int(param2);
				long_to_bin_arr(6, 10, instr.rs, instructionBin);
				long_to_bin_arr(11, 15, instr.rt, instructionBin);
				long_to_bin_arr(16, 31, 24, instructionBin);
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = MULT;
				instr.type = 2;
				sprintf(instr.toString, "MULT $%s,$%s -> 0x%s\n", param1, param2, instructionHex);
				printf("MULT $%s,$%s -> 0x%s\n", param1, param2, instructionHex);
			}
			else if(!strcmp(bloc, "NOP")){
				bin_zero(instructionBin);
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = NOP;
				instr.type = 2;
				sprintf(instr.toString, "NOP -> 0x%s\n", instructionHex);
				printf("NOP -> 0x%s\n", instructionHex);
			}
			else if(!strcmp(bloc, "OR")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , $%s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				}
				bin_zero(instructionBin);
				instr.rd = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.rt = register_string_to_int(param3);
				long_to_bin_arr(16, 20, instr.rd, instructionBin);
				long_to_bin_arr(6, 10, instr.rs, instructionBin);
				long_to_bin_arr(11, 15, instr.rt, instructionBin);
				long_to_bin_arr(21, 31, 37, instructionBin);
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = OR;
				instr.type = 2;
				sprintf(instr.toString, "OR $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instructionHex);
				printf("OR $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "ROTR")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , %s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				}
				bin_zero(instructionBin);
				instr.rd = register_string_to_int(param1);
				instr.rt = register_string_to_int(param2);
				instr.sa = register_string_to_int(param3);
				long_to_bin_arr(16, 20, instr.rd, instructionBin);
				long_to_bin_arr(11, 15, instr.rt, instructionBin);
				long_to_bin_arr(21, 25, instr.sa, instructionBin);
				long_to_bin_arr(10, 10, 1, instructionBin);
				long_to_bin_arr(26, 31, 2, instructionBin);
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = ROTR;
				instr.type = 2;
				sprintf(instr.toString, "ROTR $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
				printf("ROTR $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "SLL")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , %s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				}
				bin_zero(instructionBin);
				instr.rd = register_string_to_int(param1);
				instr.rt = register_string_to_int(param2);
				instr.sa = register_string_to_int(param3);
				long_to_bin_arr(16, 20, instr.rd, instructionBin);
				long_to_bin_arr(11, 15, instr.rt, instructionBin);
				long_to_bin_arr(21, 25, instr.sa, instructionBin);
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = SLL;
				instr.type = 2;
				sprintf(instr.toString, "SLL $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
				printf("SLL $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "SLT")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , $%s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				}
				bin_zero(instructionBin);
				instr.rd = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.rt = register_string_to_int(param3);
				long_to_bin_arr(16, 20, instr.rd, instructionBin);
				long_to_bin_arr(6, 10, instr.rs, instructionBin);
				long_to_bin_arr(11, 15, instr.rt, instructionBin);
				long_to_bin_arr(21, 31, 42, instructionBin);
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = SLT;
				instr.type = 2;
				sprintf(instr.toString, "SLT $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instructionHex);
				printf("SLT $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "SRL")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , %s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				}
				bin_zero(instructionBin);
				instr.rd = register_string_to_int(param1);
				instr.rt = register_string_to_int(param2);
				instr.sa = register_string_to_int(param3);
				long_to_bin_arr(16, 20, instr.rd, instructionBin);
				long_to_bin_arr(11, 15, instr.rt, instructionBin);
				long_to_bin_arr(21, 25, instr.sa, instructionBin);
				long_to_bin_arr(26, 31, 2, instructionBin);
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = SRL;
				instr.type = 2;
				sprintf(instr.toString, "SRL $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
				printf("SRL $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "SUB")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , $%s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				}
				bin_zero(instructionBin);
				instr.rd = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.rt = register_string_to_int(param3);
				long_to_bin_arr(16, 20, instr.rd, instructionBin);
				long_to_bin_arr(6, 10, instr.rs, instructionBin);
				long_to_bin_arr(11, 15, instr.rt, instructionBin);
				long_to_bin_arr(21, 31, 34, instructionBin);
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = SUB;
				instr.type = 2;
				sprintf(instr.toString, "SUB $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instructionHex);
				printf("SUB $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "SW")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , %[^(] ($%[^)])", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , %[^(] ($%[^)]) ", param1, param2, param3);
				}
				if(param2[0] == '-'){
					signe = 1;
					remove_sign(param2);
				}
				bin_zero(instructionBin);
				instr.rt = register_string_to_int(param1);
				instr.offset = register_string_to_int(param2);
				instr.base = register_string_to_int(param3);
				long_to_bin_arr(11, 15, instr.rt, instructionBin);
				long_to_bin_arr(16, 31, instr.offset, instructionBin);
				long_to_bin_arr(6, 10, instr.base, instructionBin);
				long_to_bin_arr(0, 5, 43, instructionBin);
				if(signe){
					instr.offset = instr.offset * -1;
					bin_twos_complement(16, 31, instructionBin);
					signe = 0;
				}
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = SW;
				instr.type = 2;
				sprintf(instr.toString, "SW $%s,%s($%s) -> 0x%s\n", param1, param2, param3, instructionHex);
				printf("SW $%s,%s($%s) -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "XOR")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , $%s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				}
				bin_zero(instructionBin);
				instr.rd = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.rt = register_string_to_int(param3);
				long_to_bin_arr(16, 20, instr.rd, instructionBin);
				long_to_bin_arr(6, 10, instr.rs, instructionBin);
				long_to_bin_arr(11, 15, instr.rt, instructionBin);
				long_to_bin_arr(21, 31, 38, instructionBin);
				bin_arr_to_hex_arr(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = XOR;
				instr.type = 2;
				sprintf(instr.toString, "XOR $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instructionHex);
				printf("XOR $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
		}
	}
	else{
		instr.type = -1;
		if(mode == 0){
			fscanf(stdin, " %[^\n]\n", bloc);
		}
		else if(mode == 1){
			fscanf(fichier, " %[^\n]\n", bloc);
		}
	}
	return(instr);
}