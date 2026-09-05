#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "memoire_mips.h"
#include "registres_mips.h"

int pasAPas = 0;

int memoire[20];
long registres[35];

typedef struct instruction instruction;

struct instruction{
	int type;          	
	int etiq;
	int opcode;
	int rs;
	int rt;
	int rd;
	int sa;
	int offset;
	int base;
	int immediate;
	int target;
	char instrHex[9];
	int exit;
};

void instructionADD(instruction instr, int printable){
	long res;
	res = lire_registre(instr.rs) + lire_registre(instr.rt);
	ecrire_registre(instr.rd, res);
	ecrire_registre(34, lire_registre(34) + 1);
	if(printable){
		printf("ADD $%d,$%d,$%d -> 0x%s\n", instr.rd, instr.rs, instr.rt, instr.instrHex);
	}
}

void instructionADDI(instruction instr, int printable){
	long res;
	res = lire_registre(instr.rs) + instr.immediate;
	ecrire_registre(instr.rt, res);
	ecrire_registre(34, lire_registre(34) + 1);
	if(printable){
		printf("ADDI $%d,$%d,%d -> 0x%s\n", instr.rt, instr.rs, instr.immediate, instr.instrHex);
	}
}

void instructionAND(instruction instr, int printable){
	long res;
	res = lire_registre(instr.rs) & lire_registre(instr.rt);
	ecrire_registre(instr.rd, res);
	ecrire_registre(34, lire_registre(34) + 1);
	if(printable){
		printf("AND $%d,$%d,$%d -> 0x%s\n", instr.rd, instr.rs, instr.rt, instr.instrHex);
	}
}

void instructionBEQ(instruction instr, int printable){
	if(lire_registre(instr.rs) == lire_registre(instr.rt)){
		ecrire_registre(34, lire_registre(34) + instr.offset);
	}
	else{
		ecrire_registre(34, lire_registre(34) + 1);
	}
	if(printable){
		printf("BEQ $%d,$%d,%d -> 0x%s\n", instr.rs, instr.rt, instr.offset, instr.instrHex);
	}
}

void instructionBGTZ(instruction instr, int printable){
	if(lire_registre(instr.rs) > 0){
		ecrire_registre(34, lire_registre(34) + instr.offset);
	}
	else{
		ecrire_registre(34, lire_registre(34) + 1);
	}
	if(printable){
		printf("BGTZ $%d,%d -> 0x%s\n", instr.rs, instr.offset, instr.instrHex);
	}
}

void instructionBLEZ(instruction instr, int printable){
	if(lire_registre(instr.rs) <= 0){
		ecrire_registre(34, lire_registre(34) + instr.offset);
	}
	else{
		ecrire_registre(34, lire_registre(34) + 1);
	}
	if(printable){
		printf("BLEZ $%d,%d -> 0x%s\n", instr.rs, instr.offset, instr.instrHex);
	}
}

void instructionBNE(instruction instr, int printable){
	if(lire_registre(instr.rs) != lire_registre(instr.rt)){
		ecrire_registre(34, lire_registre(34) + instr.offset);
	}
	else{
		ecrire_registre(34, lire_registre(34) + 1);
	}
	if(printable){
		printf("BNE $%d,$%d,%d -> 0x%s\n", instr.rs, instr.rt, instr.offset, instr.instrHex);
	}
}

void instructionDIV(instruction instr, int printable){
	long resHI, resLO;
	if(lire_registre(instr.rt) != 0){
		resHI = lire_registre(instr.rs) / lire_registre(instr.rt);
		resLO = lire_registre(instr.rs) % lire_registre(instr.rt);
		ecrire_registre(32, resHI);
		ecrire_registre(33, resLO);
	}
	ecrire_registre(34, lire_registre(34) + 1);
	if(printable){
		printf("DIV $%d,$%d -> 0x%s\n", instr.rs, instr.rt, instr.instrHex);
	}
}

void instructionJ(instruction instr, int printable){
	ecrire_registre(34, instr.target);
	if(printable){
		printf("J %d -> 0x%s\n", instr.target, instr.instrHex);
	}
}

void instructionJAL(instruction instr, int printable){
	ecrire_registre(31, lire_registre(34) + 1);
	ecrire_registre(34, instr.target);
	if(printable){
		printf("JAL %d -> 0x%s\n", instr.target, instr.instrHex);
	}
}

void instructionJR(instruction instr, int printable){
	ecrire_registre(34, lire_registre(instr.rs));
	if(printable){
		printf("JR $%d -> 0x%s\n", instr.rs, instr.instrHex);
	}
}

void instructionLUI(instruction instr, int printable){
	long res;
	res = lire_registre(instr.immediate) * 65536;
	ecrire_registre(instr.rt, res);
	ecrire_registre(34, lire_registre(34) + 1);
	if(printable){
		printf("LUI $%d,%d -> 0x%s\n", instr.rt, instr.immediate, instr.instrHex);
	}
}

void instructionLW(instruction instr, int printable){
	long res, mem;
	mem = lire_registre(instr.base) + instr.offset;
	if(mem >= 0 && mem < 20){
		res = lire_memoire(mem);
		ecrire_registre(instr.rt, res);
	}
	else{
		printf("L'adresse mémoire visée n'est pas correcte\n");
	}
	ecrire_registre(34, lire_registre(34) + 1);
	if(printable){
		printf("LW $%d,%d,($%d) -> 0x%s\n", instr.rt, instr.offset, instr.base, instr.instrHex);
	}
}

void instructionMFHI(instruction instr, int printable){
	long res;
	res = lire_registre(32);
	ecrire_registre(instr.rd, res);
	ecrire_registre(34, lire_registre(34) + 1);
	if(printable){
		printf("MFHI $%d -> 0x%s\n", instr.rd, instr.instrHex);
	}
}

void instructionMFLO(instruction instr, int printable){
	long res;
	res = lire_registre(33);
	ecrire_registre(instr.rd, res);
	ecrire_registre(34, lire_registre(34) + 1);
	if(printable){
		printf("MFLO $%d -> 0x%s\n", instr.rd, instr.instrHex);
	}
}

void instructionMULT(instruction instr, int printable){
	long resHI, resLO;
	resLO = (lire_registre(instr.rs) * lire_registre(instr.rt)) & 65535;
	resHI = lire_registre(instr.rs) * lire_registre(instr.rt) - resLO;
	ecrire_registre(32, resHI);
	ecrire_registre(33, resLO);
	ecrire_registre(34, lire_registre(34) + 1);
	if(printable){
		printf("MULT $%d,$%d -> 0x%s\n", instr.rs, instr.rt, instr.instrHex);
	}
}

void instructionNOP(instruction instr, int printable){
	ecrire_registre(34, lire_registre(34) + 1);
	if(printable){
		printf("NOP -> 0x%s\n", instr.instrHex);
	}
}

void instructionOR(instruction instr, int printable){
	long res;
	res = lire_registre(instr.rs) | lire_registre(instr.rt);
	ecrire_registre(instr.rd, res);
	ecrire_registre(34, lire_registre(34) + 1);
	if(printable){
		printf("OR $%d,$%d,$%d -> 0x%s\n", instr.rd, instr.rs, instr.rt, instr.instrHex);
	}
}

void instructionROTR(instruction instr, int printable){
	long res;
	int d;
	d = lire_registre(instr.sa);
	res = lire_registre(instr.rt);
	res = res >> d | res << (32 - d);
	ecrire_registre(instr.rd, res);
	ecrire_registre(34, lire_registre(34) + 1);
	if(printable){
		printf("ROTR $%d,$%d,%d -> 0x%s\n", instr.rd, instr.rt, instr.sa, instr.instrHex);
	}
}

void instructionSLL(instruction instr, int printable){
	long res;
	int i, nb;
	nb = lire_registre(instr.sa);
	res = lire_registre(instr.rt);
	for(i = 0; i < nb; i++){
		res = (res * 2) & 8589934591;
	}
	ecrire_registre(instr.rd, res);
	ecrire_registre(34, lire_registre(34) + 1);
	if(printable){
		printf("SLL $%d,$%d,%d -> 0x%s\n", instr.rd, instr.rt, instr.sa, instr.instrHex);
	}
}

void instructionSLT(instruction instr, int printable){
	if(lire_registre(instr.rs) < lire_registre(instr.rt)){
		ecrire_registre(instr.rd, 1);
	}
	else{
		ecrire_registre(instr.rd, 0);
	}
	ecrire_registre(34, lire_registre(34) + 1);
	if(printable){
		printf("SLT $%d,$%d,$%d -> 0x%s\n", instr.rd, instr.rs, instr.rt, instr.instrHex);
	}
}

void instructionSRL(instruction instr, int printable){
	long res;
	int i, nb;
	nb = lire_registre(instr.sa);
	res = lire_registre(instr.rt);
	for(i = 0; i < nb; i++){
		res /= 2;
	}
	ecrire_registre(instr.rd, res);
	ecrire_registre(34, lire_registre(34) + 1);
	if(printable){
		printf("SRL $%d,$%d,%d -> 0x%s\n", instr.rd, instr.rt, instr.sa, instr.instrHex);
	}
}

void instructionSUB(instruction instr, int printable){
	long res;
	res = lire_registre(instr.rs) - lire_registre(instr.rt);
	ecrire_registre(instr.rd, res);
	ecrire_registre(34, lire_registre(34) + 1);
	if(printable){
		printf("SUB $%d,$%d,$%d -> 0x%s\n", instr.rd, instr.rs, instr.rt, instr.instrHex);
	}
}

void instructionSW(instruction instr, int printable){
	long res, mem;
	res = lire_registre(instr.rt);
	mem = lire_registre(instr.base) + instr.offset;
	if(mem >= 0 && mem < 20){
		ecrire_memoire(mem, res);
	}
	else{
		printf("L'adresse mémoire visée n'est pas correcte\n");
	}
	ecrire_registre(34, lire_registre(34) + 1);
	if(printable){
		printf("SW $%d,%d($%d) -> 0x%s\n", instr.rt, instr.offset, instr.base, instr.instrHex);
	}
}

void instructionXOR(instruction instr, int printable){
	long res;
	res = lire_registre(instr.rs) ^ lire_registre(instr.rt);
	ecrire_registre(instr.rd, res);
	ecrire_registre(34, lire_registre(34) + 1);
	if(printable){
		printf("XOR $%d,$%d,$%d -> 0x%s\n", instr.rd, instr.rs, instr.rt, instr.instrHex);
	}
}

int registreCorrespondant(char *chaine){
	int longueur = strlen(chaine);
	int i, idRegistre, r = 1;
	for(i = 0; i < longueur; i++){
		if(isalpha(chaine[i])){
			r = 0;
		}
	}
	if(r){
		idRegistre = atoi(chaine);
	}
	else{
		if(!strcmp(chaine, "zero")){
			idRegistre = 0;
		}
		else if(!strcmp(chaine, "at")){
			idRegistre = 1;
		}
		else if(!strcmp(chaine, "gp")){
			idRegistre = 28;
		}
		else if(!strcmp(chaine, "sp")){
			idRegistre = 29;
		}
		else if(!strcmp(chaine, "fp")){
			idRegistre = 30;
		}
		else if(!strcmp(chaine, "ra")){
			idRegistre = 31;
		}
		else if(chaine[0] == 'v'){
			idRegistre = 2 + chaine[1] - '0';
		}
		else if(chaine[0] == 'a'){
			idRegistre = 4 + chaine[1] - '0';
		}
		else if(chaine[0] == 's'){
			idRegistre = 16 + chaine[1] - '0';
		}
		else if(chaine[0] == 'k'){
			idRegistre = 26 + chaine[1] - '0';
		}
		else if(chaine[0] == 't'){
			if(chaine[1] - '0' <= 7){
				idRegistre = 8 + chaine[1] - '0';
			}
			else{
				idRegistre = 24 + chaine[1] - '0';
			}
		}
	}
	return(idRegistre);
}

void zeroBin(int *tab){
	int i;
	for(i = 0; i < 32; i++){
		tab[i] = 0;
	}
}

void changeBin(int debut, int fin, int valeur, int *tab){
	int i;
	long bin = 1;
	for(i = debut; i < fin; i++){
		bin *= 2;
	}
	for(i = debut; i <= fin; i++){
		if(valeur >= bin){
			valeur -= bin;
			tab[i] = 1;
		}
		bin /= 2;
	}
}

void paramNonSigne(char *chaine){
	int i = 0;
	while(chaine[i + 1] != '\0'){
		chaine[i] = chaine[i + 1];
		i++;
	}
	chaine[i] = '\0';
}

void complementDeuxBin(int debut, int fin, int *tab){
	int i;
	for(i = debut; i <= fin; i++){
		if(tab[i] == 0){
			tab[i] = 1;
		}
		else{
			tab[i] = 0;
		}
	}
	i = fin;
	while(tab[i] == 1){
		tab[i--] = 0;
	}
	tab[i] = 1;
}

void convHex(int *tabBin, char *tabHex){
	int i, j, m, v;
	for(i = 0; i < 8; i++){
		m = 8;
		v = 0;
		for(j = 0; j < 4; j++){
			v += m * tabBin[i * 4 + j];
			m /= 2;
		}
		if(v <= 9){
			tabHex[i] = '0' + v;
		}
		else{
			switch(v){
				case 10 : tabHex[i] = 'a'; break;
				case 11 : tabHex[i] = 'b'; break;
				case 12 : tabHex[i] = 'c'; break;
				case 13 : tabHex[i] = 'd'; break;
				case 14 : tabHex[i] = 'e'; break;
				case 15 : tabHex[i] = 'f'; break;
			}
		}
	}
	tabHex[8] = '\0';
}

instruction decodeInstruction(int mode, FILE *fichier){
	char bloc[50];
	char param1[20], param2[20], param3[20];
	int instructionBin[32];
	char instructionHex[9];
	int signe = 0;
	instruction *instr = malloc(sizeof(instruction));
	instr->type = 0;
	instr->etiq = 0;
	instr->opcode = 0;
	instr->rs = 0;
	instr->rt = 0;
	instr->rd = 0;
	instr->sa = 0;
	instr->offset = 0;
	instr->base = 0;
	instr->immediate = 0;
	instr->target = 0;
	instr->exit = 0;
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
				zeroBin(instructionBin);
				instr->rd = registreCorrespondant(param1);
				instr->rs = registreCorrespondant(param2);
				instr->rt = registreCorrespondant(param3);
				changeBin(16, 20, instr->rd, instructionBin);
				changeBin(6, 10, instr->rs, instructionBin);
				changeBin(11, 15, instr->rt, instructionBin);
				changeBin(21, 31, 32, instructionBin);
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 1;
				instr->type = 2;
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
					paramNonSigne(param3);
				}
				zeroBin(instructionBin);
				instr->rt = registreCorrespondant(param1);
				instr->rs = registreCorrespondant(param2);
				instr->immediate = atoi(param3);
				changeBin(11, 15, instr->rt, instructionBin);
				changeBin(6, 10, instr->rs, instructionBin);
				changeBin(16, 31, instr->immediate, instructionBin);
				changeBin(0, 5, 8, instructionBin);
				if(signe){
					instr->immediate = instr->immediate * -1;
					complementDeuxBin(16, 31, instructionBin);
					signe = 0;
				}
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 2;
				instr->type = 2;
				printf("ADDI $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "AND")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , $%s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				}
				zeroBin(instructionBin);
				instr->rd = registreCorrespondant(param1);
				instr->rs = registreCorrespondant(param2);
				instr->rt = registreCorrespondant(param3);
				changeBin(16, 20, instr->rd, instructionBin);
				changeBin(6, 10, instr->rs, instructionBin);
				changeBin(11, 15, instr->rt, instructionBin);
				changeBin(21, 31, 36, instructionBin);
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 3;
				instr->type = 2;
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
					paramNonSigne(param3);
				}
				zeroBin(instructionBin);
				instr->rt = registreCorrespondant(param1);
				instr->rs = registreCorrespondant(param2);
				instr->offset = atoi(param3);
				changeBin(6, 10, instr->rt, instructionBin);
				changeBin(11, 15, instr->rs, instructionBin);
				changeBin(16, 31, instr->offset, instructionBin);
				changeBin(0, 5, 4, instructionBin);
				if(signe){
					instr->immediate = instr->immediate * -1;
					complementDeuxBin(16, 31, instructionBin);
					signe = 0;
				}
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 4;
				instr->type = 2;
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
					paramNonSigne(param2);
				}
				zeroBin(instructionBin);
				instr->rs = registreCorrespondant(param1);
				instr->offset = atoi(param2);
				changeBin(6, 10, instr->rs, instructionBin);
				changeBin(16, 31, instr->offset, instructionBin);
				changeBin(0, 5, 7, instructionBin);
				if(signe){
					instr->offset = instr->offset * -1;
					complementDeuxBin(16, 31, instructionBin);
					signe = 0;
				}
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 5;
				instr->type = 2;
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
					paramNonSigne(param2);
				}
				zeroBin(instructionBin);
				instr->rs = registreCorrespondant(param1);
				instr->offset = atoi(param2);
				changeBin(6, 10, instr->rs, instructionBin);
				changeBin(16, 31, instr->offset, instructionBin);
				changeBin(0, 5, 6, instructionBin);
				if(signe){
					instr->offset = instr->offset * -1;
					complementDeuxBin(16, 31, instructionBin);
					signe = 0;
				}
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 6;
				instr->type = 2;
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
					paramNonSigne(param3);
				}
				zeroBin(instructionBin);
				instr->rt = registreCorrespondant(param1);
				instr->rs = registreCorrespondant(param2);
				instr->offset = atoi(param3);
				changeBin(6, 10, instr->rt, instructionBin);
				changeBin(11, 15, instr->rs, instructionBin);
				changeBin(16, 31, instr->offset, instructionBin);
				changeBin(0, 5, 5, instructionBin);
				if(signe){
					instr->offset = instr->offset * -1;
					complementDeuxBin(16, 31, instructionBin);
					signe = 0;
				}
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 7;
				instr->type = 2;
				printf("BNE $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "DIV")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%s", param1, param2);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%s ", param1, param2);
				}
				zeroBin(instructionBin);
				instr->rs = registreCorrespondant(param1);
				instr->rt = registreCorrespondant(param2);
				changeBin(6, 10, instr->rs, instructionBin);
				changeBin(11, 15, instr->rt, instructionBin);
				changeBin(16, 31, 26, instructionBin);
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 8;
				instr->type = 2;
				printf("DIV $%s,$%s -> 0x%s\n", param1, param2, instructionHex);
			}
			else if(!strcmp(bloc, "EXIT")){
				instr->exit = 1;
			}
			else if(!strcmp(bloc, "J")){
				if(mode == 0){
					fscanf(stdin, " %s", param1);
				}
				else if(mode == 1){
					fscanf(fichier, " %s ", param1);
				}
				zeroBin(instructionBin);
				instr->target = registreCorrespondant(param1);
				changeBin(6, 31, instr->target, instructionBin);
				changeBin(0, 5, 2, instructionBin);
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 9;
				instr->type = 2;
				printf("J %s -> 0x%s\n", param1, instructionHex);
			}
			else if(!strcmp(bloc, "JAL")){
				if(mode == 0){
					fscanf(stdin, " %s", param1);
				}
				else if(mode == 1){
					fscanf(fichier, " %s ", param1);
				}
				zeroBin(instructionBin);
				instr->target = registreCorrespondant(param1);
				changeBin(6, 31, instr->target, instructionBin);
				changeBin(0, 5, 3, instructionBin);
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 10;
				instr->type = 2;
				printf("JAL %s -> 0x%s\n", param1, instructionHex);
			}
			else if(!strcmp(bloc, "JR")){
				if(mode == 0){
					fscanf(stdin, " $%s", param1);
				}
				else if(mode == 1){
					fscanf(fichier, " $%s ", param1);
				}
				zeroBin(instructionBin);
				instr->rs = registreCorrespondant(param1);
				changeBin(6, 10, instr->rs, instructionBin);
				changeBin(11, 31, 8, instructionBin);
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 11;
				instr->type = 2;
				printf("JR $%s -> 0x%s\n", param1, instructionHex);
			}
			else if(!strcmp(bloc, "LUI")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , %s", param1, param2);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , %s ", param1, param2);
				}
				zeroBin(instructionBin);
				instr->rt = registreCorrespondant(param1);
				instr->immediate = registreCorrespondant(param2);
				changeBin(11, 15, instr->rt, instructionBin);
				changeBin(16, 31, instr->immediate, instructionBin);
				changeBin(0, 5, 15, instructionBin);
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 12;
				instr->type = 2;
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
					paramNonSigne(param2);
				}
				zeroBin(instructionBin);
				instr->rt = registreCorrespondant(param1);
				instr->offset = registreCorrespondant(param2);
				instr->base = registreCorrespondant(param3);
				changeBin(11, 15, instr->rt, instructionBin);
				changeBin(16, 31, instr->offset, instructionBin);
				changeBin(6, 10, instr->base, instructionBin);
				changeBin(0, 5, 35, instructionBin);
				if(signe){
					instr->offset = instr->offset * -1;
					complementDeuxBin(16, 31, instructionBin);
					signe = 0;
				}
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 13;
				instr->type = 2;
				printf("LW $%s,%s($%s) -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "MFHI")){
				if(mode == 0){
					fscanf(stdin, " $%s", param1);
				}
				else if(mode == 1){
					fscanf(fichier, " $%s ", param1);
				}
				zeroBin(instructionBin);
				instr->rd = registreCorrespondant(param1);
				changeBin(16, 20, instr->rd, instructionBin);
				changeBin(21, 31, 16, instructionBin);
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 14;
				instr->type = 2;
				printf("MFHI $%s -> 0x%s\n", param1, instructionHex);
			}
			else if(!strcmp(bloc, "MFLO")){
				if(mode == 0){
					fscanf(stdin, " $%s", param1);
				}
				else if(mode == 1){
					fscanf(fichier, " $%s ", param1);
				}
				zeroBin(instructionBin);
				instr->rd = registreCorrespondant(param1);
				changeBin(16, 20, instr->rd, instructionBin);
				changeBin(21, 31, 18, instructionBin);
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 15;
				instr->type = 2;
				printf("MFLO $%s -> 0x%s\n", param1, instructionHex);
			}
			else if(!strcmp(bloc, "MULT")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%s", param1, param2);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%s ", param1, param2);
				}
				zeroBin(instructionBin);
				instr->rs = registreCorrespondant(param1);
				instr->rt = registreCorrespondant(param2);
				changeBin(6, 10, instr->rs, instructionBin);
				changeBin(11, 15, instr->rt, instructionBin);
				changeBin(16, 31, 24, instructionBin);
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 16;
				instr->type = 2;
				printf("MULT $%s,$%s -> 0x%s\n", param1, param2, instructionHex);
			}
			else if(!strcmp(bloc, "NOP")){
				zeroBin(instructionBin);
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 17;
				instr->type = 2;
				printf("NOP -> 0x%s\n", instructionHex);
			}
			else if(!strcmp(bloc, "OR")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , $%s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				}
				zeroBin(instructionBin);
				instr->rd = registreCorrespondant(param1);
				instr->rs = registreCorrespondant(param2);
				instr->rt = registreCorrespondant(param3);
				changeBin(16, 20, instr->rd, instructionBin);
				changeBin(6, 10, instr->rs, instructionBin);
				changeBin(11, 15, instr->rt, instructionBin);
				changeBin(21, 31, 37, instructionBin);
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 18;
				instr->type = 2;
				printf("OR $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "ROTR")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , %s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				}
				zeroBin(instructionBin);
				instr->rd = registreCorrespondant(param1);
				instr->rt = registreCorrespondant(param2);
				instr->sa = registreCorrespondant(param3);
				changeBin(16, 20, instr->rd, instructionBin);
				changeBin(11, 15, instr->rt, instructionBin);
				changeBin(21, 25, instr->sa, instructionBin);
				changeBin(10, 10, 1, instructionBin);
				changeBin(26, 31, 2, instructionBin);
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 19;
				instr->type = 2;
				printf("ROTR $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "SLL")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , %s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				}
				zeroBin(instructionBin);
				instr->rd = registreCorrespondant(param1);
				instr->rt = registreCorrespondant(param2);
				instr->sa = registreCorrespondant(param3);
				changeBin(16, 20, instr->rd, instructionBin);
				changeBin(11, 15, instr->rt, instructionBin);
				changeBin(21, 25, instr->sa, instructionBin);
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 20;
				instr->type = 2;
				printf("SLL $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "SLT")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , $%s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				}
				zeroBin(instructionBin);
				instr->rd = registreCorrespondant(param1);
				instr->rs = registreCorrespondant(param2);
				instr->rt = registreCorrespondant(param3);
				changeBin(16, 20, instr->rd, instructionBin);
				changeBin(6, 10, instr->rs, instructionBin);
				changeBin(11, 15, instr->rt, instructionBin);
				changeBin(21, 31, 42, instructionBin);
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 21;
				instr->type = 2;
				printf("SLT $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "SRL")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , %s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , %s ", param1, param2, param3);
				}
				zeroBin(instructionBin);
				instr->rd = registreCorrespondant(param1);
				instr->rt = registreCorrespondant(param2);
				instr->sa = registreCorrespondant(param3);
				changeBin(16, 20, instr->rd, instructionBin);
				changeBin(11, 15, instr->rt, instructionBin);
				changeBin(21, 25, instr->sa, instructionBin);
				changeBin(26, 31, 2, instructionBin);
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 22;
				instr->type = 2;
				printf("SRL $%s,$%s,%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "SUB")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , $%s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				}
				zeroBin(instructionBin);
				instr->rd = registreCorrespondant(param1);
				instr->rs = registreCorrespondant(param2);
				instr->rt = registreCorrespondant(param3);
				changeBin(16, 20, instr->rd, instructionBin);
				changeBin(6, 10, instr->rs, instructionBin);
				changeBin(11, 15, instr->rt, instructionBin);
				changeBin(21, 31, 34, instructionBin);
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 23;
				instr->type = 2;
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
					paramNonSigne(param2);
				}
				zeroBin(instructionBin);
				instr->rt = registreCorrespondant(param1);
				instr->offset = registreCorrespondant(param2);
				instr->base = registreCorrespondant(param3);
				changeBin(11, 15, instr->rt, instructionBin);
				changeBin(16, 31, instr->offset, instructionBin);
				changeBin(6, 10, instr->base, instructionBin);
				changeBin(0, 5, 43, instructionBin);
				if(signe){
					instr->offset = instr->offset * -1;
					complementDeuxBin(16, 31, instructionBin);
					signe = 0;
				}
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 24;
				instr->type = 2;
				printf("SW $%s,%s($%s) -> 0x%s\n", param1, param2, param3, instructionHex);
			}
			else if(!strcmp(bloc, "XOR")){
				if(mode == 0){
					fscanf(stdin, " $%[^,] , $%[^,] , $%s", param1, param2, param3);
				}
				else if(mode == 1){
					fscanf(fichier, " $%[^,] , $%[^,] , $%s ", param1, param2, param3);
				}
				zeroBin(instructionBin);
				instr->rd = registreCorrespondant(param1);
				instr->rs = registreCorrespondant(param2);
				instr->rt = registreCorrespondant(param3);
				changeBin(16, 20, instr->rd, instructionBin);
				changeBin(6, 10, instr->rs, instructionBin);
				changeBin(11, 15, instr->rt, instructionBin);
				changeBin(21, 31, 38, instructionBin);
				convHex(instructionBin, instructionHex);
				strcpy(instr->instrHex, instructionHex);
				instr->opcode = 25;
				instr->type = 2;
				printf("XOR $%s,$%s,$%s -> 0x%s\n", param1, param2, param3, instructionHex);
			}
		}
	}
	else{
		instr->type = -1;
		if(mode == 0){
			fscanf(stdin, " %[^\n]\n", bloc);
		}
		else if(mode == 1){
			fscanf(fichier, " %[^\n]\n", bloc);
		}
	}
	return(*instr);
}

void afficheurMemoireRegistres(){
	int i, j, v;
	char carHex[4] = {'0', '4', '8', 'C'};
	printf("\n");
	printf("------------ Affichage des états des registres -----------\n");
	for(i = 0; i < 8; i++){
		for(j = 0; j < 4; j++){
			v = lire_registre(4 * i + j);
			printf("$%s%d : %-10d ", 4 * i + j <= 9 ? "0" : "", 4 * i + j, v);
		}
		printf("\n");
	}
	printf("                 HI  : %-10d LO  : %-10d\n", lire_registre(32), lire_registre(33));
	printf("\n");
	printf("-------------------------- Affichage des états de la mémoire -------------------------\n");
	for(i = 0; i < 5; i++){
		for(j = 0; j < 4; j++){
			v = lire_memoire(4 * i + j);
			printf("@0000 00%d%c : %-10d ", i, carHex[j], v);
		}
		printf("\n");
	}
	printf("\n");
}

void modeInteractif(){
	instruction *instructionEnCours = malloc(sizeof(instruction));
	instructionEnCours->exit = 0;
	initialiser_registres();
	initialiser_memoire();
	while(instructionEnCours->exit == 0){
		printf("Entrez votre instruction :\n");
		*instructionEnCours = decodeInstruction(0, NULL);
		if(instructionEnCours->exit == 0){
			switch(instructionEnCours->opcode){
				case 1 :
					instructionADD(*instructionEnCours, 0);
					break;
				case 2 :
					instructionADDI(*instructionEnCours, 0);
					break;
				case 3 :
					instructionAND(*instructionEnCours, 0);
					break;
				case 4 : 
					instructionBEQ(*instructionEnCours, 0);
					break;
				case 5 : 
					instructionBGTZ(*instructionEnCours, 0);
					break;
				case 6 : 
					instructionBLEZ(*instructionEnCours, 0);
					break;
				case 7 : 
					instructionBNE(*instructionEnCours, 0);
					break;
				case 8 : 
					instructionDIV(*instructionEnCours, 0);
					break;
				case 9 : 
					instructionJ(*instructionEnCours, 0);
					break;
				case 10 : 
					instructionJAL(*instructionEnCours, 0);
					break;
				case 11 : 
					instructionJR(*instructionEnCours, 0);
					break;
				case 12 :
					instructionLUI(*instructionEnCours, 0);
					break;
				case 13 :
					instructionLW(*instructionEnCours, 0);
					break;
				case 14 :
					instructionMFHI(*instructionEnCours, 0);
					break;
				case 15 : 
					instructionMFLO(*instructionEnCours, 0);
					break;
				case 16 : 
					instructionMULT(*instructionEnCours, 0);
					break;
				case 17 : 
					instructionNOP(*instructionEnCours, 0);
					break;
				case 18 : 
					instructionOR(*instructionEnCours, 0);
					break;
				case 19 : 
					instructionROTR(*instructionEnCours, 0);
					break;
				case 20 : 
					instructionSLL(*instructionEnCours, 0);
					break;
				case 21 : 
					instructionSLT(*instructionEnCours, 0);
					break;
				case 22 : 
					instructionSRL(*instructionEnCours, 0);
					break;
				case 23 : 
					instructionSUB(*instructionEnCours, 0);
					break;
				case 24 : 
					instructionSW(*instructionEnCours, 0);
					break;
				case 25 : 
					instructionXOR(*instructionEnCours, 0);
					break;
				default:
					break;
			}
			afficheurMemoireRegistres();
		}
	}
}

void modeNonInteractif(char *fichierR, char *fichierS1, char *fichierS2){
	int i = 0, n, erreur;
	char chaine[30];
	char espace[2];
	instruction tabInstructions[100];
	FILE *fr, *fs1, *fs2;
	if(pasAPas){
		if(((fr = fopen(fichierR, "r")) == NULL)){
			printf("Erreur d'ouverture du fichier\n");
			erreur = 1;
		}
	}
	if(!pasAPas){
		if(((fr = fopen(fichierR, "r")) == NULL) || ((fs1 = fopen(fichierS1, "w")) == NULL) || ((fs2 = fopen(fichierS2, "w")) == NULL)){
			printf("Erreur d'ouverture du/des fichier(s)\n");
			erreur = 1;
		}
	}
	if(!erreur){
		printf("\n----Instruction Decode----\n");
		while(!feof(fr)){
			tabInstructions[i] = decodeInstruction(1, fr);
			if(tabInstructions[i].opcode > 0 && tabInstructions[i].opcode <= 25){
				i++;
			}
		}
		printf("\n---------Execute---------\n");
		n = i;
		if(!pasAPas){
			for(i = 0; i < n; i++){
				fputs(tabInstructions[i].instrHex, fs1);
				fputc(13, fs1);
			}
		}
		i = 0;
		while(i < n){
			switch(tabInstructions[i].opcode){
				case 1 :
					instructionADD(tabInstructions[i], 1);
					break;
				case 2 :
					instructionADDI(tabInstructions[i], 1);
					break;
				case 3 :
					instructionAND(tabInstructions[i], 1);
					break;
				case 4 : 
					instructionBEQ(tabInstructions[i], 1);
					break;
				case 5 : 
					instructionBGTZ(tabInstructions[i], 1);
					break;
				case 6 : 
					instructionBLEZ(tabInstructions[i], 1);
					break;
				case 7 : 
					instructionBNE(tabInstructions[i], 1);
					break;
				case 8 : 
					instructionDIV(tabInstructions[i], 1);
					break;
				case 9 : 
					instructionJ(tabInstructions[i], 1);
					break;
				case 10 : 
					instructionJAL(tabInstructions[i], 1);
					break;
				case 11 : 
					instructionJR(tabInstructions[i], 1);
					break;
				case 12 :
					instructionLUI(tabInstructions[i], 1);
					break;
				case 13 :
					instructionLW(tabInstructions[i], 1);
					break;
				case 14 :
					instructionMFHI(tabInstructions[i], 1);
					break;
				case 15 : 
					instructionMFLO(tabInstructions[i], 1);
					break;
				case 16 : 
					instructionMULT(tabInstructions[i], 1);
					break;
				case 17 : 
					instructionNOP(tabInstructions[i], 1);
					break;
				case 18 : 
					instructionOR(tabInstructions[i], 1);
					break;
				case 19 : 
					instructionROTR(tabInstructions[i], 1);
					break;
				case 20 : 
					instructionSLL(tabInstructions[i], 1);
					break;
				case 21 : 
					instructionSLT(tabInstructions[i], 1);
					break;
				case 22 : 
					instructionSRL(tabInstructions[i], 1);
					break;
				case 23 : 
					instructionSUB(tabInstructions[i], 1);
					break;
				case 24 : 
					instructionSW(tabInstructions[i], 1);
					break;
				case 25 : 
					instructionXOR(tabInstructions[i], 1);
					break;
				default:
					break;
			}
			if(pasAPas){
				afficheurMemoireRegistres();
				fgets(espace, sizeof(espace), stdin);
			}
			i = lire_registre(34);
		}
		if(!pasAPas){
			afficheurMemoireRegistres();
			for(i = 0; i < 34; i++){
				if(i <= 31){
					sprintf(chaine, "$%d : %d", i, lire_registre(i));
					fputs(chaine, fs2);
					fputc(13, fs2);
				}
				else if(i == 32){
					sprintf(chaine, "HI : %d", lire_registre(i));
					fputs(chaine, fs2);
					fputc(13, fs2);
				}
				else{
					sprintf(chaine, "LO : %d", lire_registre(i));
					fputs(chaine, fs2);
					fputc(13, fs2);
				}
			}
		}
	}
}

int main(int argc, char *argv[]){
	if(argc == 1){
		modeInteractif();
	}
	else if(argc == 3 && !strcmp(argv[2], "-pas")){
		pasAPas = 1;
		modeNonInteractif(argv[1], NULL, NULL);
	}
	else if(argc == 4){
		modeNonInteractif(argv[1], argv[2], argv[3]);
	}
	return(0);
}