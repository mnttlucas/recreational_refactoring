#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decode.h"
#include "utils.h"

instruction decode_instruction(int mode, FILE *fichier){
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
				binary_zero(instructionBin);
				instr.rd = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.rt = register_string_to_int(param3);
				changeBin(16, 20, instr.rd, instructionBin);
				changeBin(6, 10, instr.rs, instructionBin);
				changeBin(11, 15, instr.rt, instructionBin);
				changeBin(21, 31, 32, instructionBin);
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 1;
				instr.type = 2;
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
				binary_zero(instructionBin);
				instr.rt = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.immediate = atoi(param3);
				changeBin(11, 15, instr.rt, instructionBin);
				changeBin(6, 10, instr.rs, instructionBin);
				changeBin(16, 31, instr.immediate, instructionBin);
				changeBin(0, 5, 8, instructionBin);
				if(signe){
					instr.immediate = instr.immediate * -1;
					binary_twos_complement(16, 31, instructionBin);
					signe = 0;
				}
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 2;
				instr.type = 2;
				printf("ADDI $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "AND")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , $%s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				}
				binary_zero(instructionBin);
				instr.rd = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.rt = register_string_to_int(param3);
				changeBin(16, 20, instr.rd, instructionBin);
				changeBin(6, 10, instr.rs, instructionBin);
				changeBin(11, 15, instr.rt, instructionBin);
				changeBin(21, 31, 36, instructionBin);
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 3;
				instr.type = 2;
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
				binary_zero(instructionBin);
				instr.rt = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.offset = atoi(param3);
				changeBin(6, 10, instr.rt, instructionBin);
				changeBin(11, 15, instr.rs, instructionBin);
				changeBin(16, 31, instr.offset, instructionBin);
				changeBin(0, 5, 4, instructionBin);
				if(signe){
					instr.immediate = instr.immediate * -1;
					binary_twos_complement(16, 31, instructionBin);
					signe = 0;
				}
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 4;
				instr.type = 2;
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
				binary_zero(instructionBin);
				instr.rs = register_string_to_int(param1);
				instr.offset = atoi(param2);
				changeBin(6, 10, instr.rs, instructionBin);
				changeBin(16, 31, instr.offset, instructionBin);
				changeBin(0, 5, 7, instructionBin);
				if(signe){
					instr.offset = instr.offset * -1;
					binary_twos_complement(16, 31, instructionBin);
					signe = 0;
				}
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 5;
				instr.type = 2;
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
				binary_zero(instructionBin);
				instr.rs = register_string_to_int(param1);
				instr.offset = atoi(param2);
				changeBin(6, 10, instr.rs, instructionBin);
				changeBin(16, 31, instr.offset, instructionBin);
				changeBin(0, 5, 6, instructionBin);
				if(signe){
					instr.offset = instr.offset * -1;
					binary_twos_complement(16, 31, instructionBin);
					signe = 0;
				}
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 6;
				instr.type = 2;
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
				binary_zero(instructionBin);
				instr.rt = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.offset = atoi(param3);
				changeBin(6, 10, instr.rt, instructionBin);
				changeBin(11, 15, instr.rs, instructionBin);
				changeBin(16, 31, instr.offset, instructionBin);
				changeBin(0, 5, 5, instructionBin);
				if(signe){
					instr.offset = instr.offset * -1;
					binary_twos_complement(16, 31, instructionBin);
					signe = 0;
				}
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 7;
				instr.type = 2;
				printf("BNE $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "DIV")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%s", param1, param2);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%s ", param1, param2);
				}
				binary_zero(instructionBin);
				instr.rs = register_string_to_int(param1);
				instr.rt = register_string_to_int(param2);
				changeBin(6, 10, instr.rs, instructionBin);
				changeBin(11, 15, instr.rt, instructionBin);
				changeBin(16, 31, 26, instructionBin);
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 8;
				instr.type = 2;
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
				binary_zero(instructionBin);
				instr.target = register_string_to_int(param1);
				changeBin(6, 31, instr.target, instructionBin);
				changeBin(0, 5, 2, instructionBin);
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 9;
				instr.type = 2;
				printf("J %s -> 0x%s\n", param1, instructionHex);
			}
			else if(!strcmp(bloc, "JAL")){
				if(mode == 0){
					fscanf(stdin, " %s", param1);
				}
				else if(mode == 1){
					fscanf(fichier, " %s ", param1);
				}
				binary_zero(instructionBin);
				instr.target = register_string_to_int(param1);
				changeBin(6, 31, instr.target, instructionBin);
				changeBin(0, 5, 3, instructionBin);
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 10;
				instr.type = 2;
				printf("JAL %s -> 0x%s\n", param1, instructionHex);
			}
			else if(!strcmp(bloc, "JR")){
				if(mode == 0){
					fscanf(stdin, " $%s", param1);
				}
				else if(mode == 1){
					fscanf(fichier, " $%s ", param1);
				}
				binary_zero(instructionBin);
				instr.rs = register_string_to_int(param1);
				changeBin(6, 10, instr.rs, instructionBin);
				changeBin(11, 31, 8, instructionBin);
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 11;
				instr.type = 2;
				printf("JR $%s -> 0x%s\n", param1, instructionHex);
			}
			else if(!strcmp(bloc, "LUI")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , %s", param1, param2);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , %s ", param1, param2);
				}
				binary_zero(instructionBin);
				instr.rt = register_string_to_int(param1);
				instr.immediate = register_string_to_int(param2);
				changeBin(11, 15, instr.rt, instructionBin);
				changeBin(16, 31, instr.immediate, instructionBin);
				changeBin(0, 5, 15, instructionBin);
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 12;
				instr.type = 2;
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
				binary_zero(instructionBin);
				instr.rt = register_string_to_int(param1);
				instr.offset = register_string_to_int(param2);
				instr.base = register_string_to_int(param3);
				changeBin(11, 15, instr.rt, instructionBin);
				changeBin(16, 31, instr.offset, instructionBin);
				changeBin(6, 10, instr.base, instructionBin);
				changeBin(0, 5, 35, instructionBin);
				if(signe){
					instr.offset = instr.offset * -1;
					binary_twos_complement(16, 31, instructionBin);
					signe = 0;
				}
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 13;
				instr.type = 2;
				printf("LW $%s,%s($%s) -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "MFHI")){
				if(mode == 0){
					fscanf(stdin, " $%s", param1);
				}
				else if(mode == 1){
					fscanf(fichier, " $%s ", param1);
				}
				binary_zero(instructionBin);
				instr.rd = register_string_to_int(param1);
				changeBin(16, 20, instr.rd, instructionBin);
				changeBin(21, 31, 16, instructionBin);
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 14;
				instr.type = 2;
				printf("MFHI $%s -> 0x%s\n", param1, instructionHex);
			}
			else if(!strcmp(bloc, "MFLO")){
				if(mode == 0){
					fscanf(stdin, " $%s", param1);
				}
				else if(mode == 1){
					fscanf(fichier, " $%s ", param1);
				}
				binary_zero(instructionBin);
				instr.rd = register_string_to_int(param1);
				changeBin(16, 20, instr.rd, instructionBin);
				changeBin(21, 31, 18, instructionBin);
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 15;
				instr.type = 2;
				printf("MFLO $%s -> 0x%s\n", param1, instructionHex);
			}
			else if(!strcmp(bloc, "MULT")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%s", param1, param2);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%s ", param1, param2);
				}
				binary_zero(instructionBin);
				instr.rs = register_string_to_int(param1);
				instr.rt = register_string_to_int(param2);
				changeBin(6, 10, instr.rs, instructionBin);
				changeBin(11, 15, instr.rt, instructionBin);
				changeBin(16, 31, 24, instructionBin);
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 16;
				instr.type = 2;
				printf("MULT $%s,$%s -> 0x%s\n", param1, param2, instructionHex);
			}
			else if(!strcmp(bloc, "NOP")){
				binary_zero(instructionBin);
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 17;
				instr.type = 2;
				printf("NOP -> 0x%s\n", instructionHex);
			}
			else if(!strcmp(bloc, "OR")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , $%s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				}
				binary_zero(instructionBin);
				instr.rd = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.rt = register_string_to_int(param3);
				changeBin(16, 20, instr.rd, instructionBin);
				changeBin(6, 10, instr.rs, instructionBin);
				changeBin(11, 15, instr.rt, instructionBin);
				changeBin(21, 31, 37, instructionBin);
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 18;
				instr.type = 2;
				printf("OR $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "ROTR")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , %s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				}
				binary_zero(instructionBin);
				instr.rd = register_string_to_int(param1);
				instr.rt = register_string_to_int(param2);
				instr.sa = register_string_to_int(param3);
				changeBin(16, 20, instr.rd, instructionBin);
				changeBin(11, 15, instr.rt, instructionBin);
				changeBin(21, 25, instr.sa, instructionBin);
				changeBin(10, 10, 1, instructionBin);
				changeBin(26, 31, 2, instructionBin);
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 19;
				instr.type = 2;
				printf("ROTR $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "SLL")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , %s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				}
				binary_zero(instructionBin);
				instr.rd = register_string_to_int(param1);
				instr.rt = register_string_to_int(param2);
				instr.sa = register_string_to_int(param3);
				changeBin(16, 20, instr.rd, instructionBin);
				changeBin(11, 15, instr.rt, instructionBin);
				changeBin(21, 25, instr.sa, instructionBin);
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 20;
				instr.type = 2;
				printf("SLL $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "SLT")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , $%s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				}
				binary_zero(instructionBin);
				instr.rd = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.rt = register_string_to_int(param3);
				changeBin(16, 20, instr.rd, instructionBin);
				changeBin(6, 10, instr.rs, instructionBin);
				changeBin(11, 15, instr.rt, instructionBin);
				changeBin(21, 31, 42, instructionBin);
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 21;
				instr.type = 2;
				printf("SLT $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "SRL")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , %s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				}
				binary_zero(instructionBin);
				instr.rd = register_string_to_int(param1);
				instr.rt = register_string_to_int(param2);
				instr.sa = register_string_to_int(param3);
				changeBin(16, 20, instr.rd, instructionBin);
				changeBin(11, 15, instr.rt, instructionBin);
				changeBin(21, 25, instr.sa, instructionBin);
				changeBin(26, 31, 2, instructionBin);
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 22;
				instr.type = 2;
				printf("SRL $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "SUB")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , $%s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				}
				binary_zero(instructionBin);
				instr.rd = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.rt = register_string_to_int(param3);
				changeBin(16, 20, instr.rd, instructionBin);
				changeBin(6, 10, instr.rs, instructionBin);
				changeBin(11, 15, instr.rt, instructionBin);
				changeBin(21, 31, 34, instructionBin);
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 23;
				instr.type = 2;
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
				binary_zero(instructionBin);
				instr.rt = register_string_to_int(param1);
				instr.offset = register_string_to_int(param2);
				instr.base = register_string_to_int(param3);
				changeBin(11, 15, instr.rt, instructionBin);
				changeBin(16, 31, instr.offset, instructionBin);
				changeBin(6, 10, instr.base, instructionBin);
				changeBin(0, 5, 43, instructionBin);
				if(signe){
					instr.offset = instr.offset * -1;
					binary_twos_complement(16, 31, instructionBin);
					signe = 0;
				}
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 24;
				instr.type = 2;
				printf("SW $%s,%s($%s) -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "XOR")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , $%s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				}
				binary_zero(instructionBin);
				instr.rd = register_string_to_int(param1);
				instr.rs = register_string_to_int(param2);
				instr.rt = register_string_to_int(param3);
				changeBin(16, 20, instr.rd, instructionBin);
				changeBin(6, 10, instr.rs, instructionBin);
				changeBin(11, 15, instr.rt, instructionBin);
				changeBin(21, 31, 38, instructionBin);
				binary_to_hex(instructionBin, instructionHex);
				strcpy(instr.instrHex, instructionHex);
				instr.opcode = 25;
				instr.type = 2;
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