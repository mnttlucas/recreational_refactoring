//#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "cpu.h"
#include "decode.h"
#include "execute.h"
#include "instruction.h"
#include "memory.h"
#include "registers.h"
#include "utils.h"

void interactive_mode(CPU *cpu, config *config){
	instruction instructionEnCours;
	instructionEnCours.exit = 0;
	config->verbose = 0;
	registers_init(cpu);
	memory_init(cpu);
	while(instructionEnCours.exit == 0){
		printf("Entrez votre instruction :\n");
		instructionEnCours = decode_instruction(0, NULL);
		if(instructionEnCours.exit == 0){
			switch(instructionEnCours.opcode){
				case 1 :
					instructionADD(cpu, instructionEnCours, 0);
					break;
				case 2 :
					instructionADDI(cpu, instructionEnCours, 0);
					break;
				case 3 :
					instructionAND(cpu, instructionEnCours, 0);
					break;
				case 4 : 
					instructionBEQ(cpu, instructionEnCours, 0);
					break;
				case 5 : 
					instructionBGTZ(cpu, instructionEnCours, 0);
					break;
				case 6 : 
					instructionBLEZ(cpu, instructionEnCours, 0);
					break;
				case 7 : 
					instructionBNE(cpu, instructionEnCours, 0);
					break;
				case 8 : 
					instructionDIV(cpu, instructionEnCours, 0);
					break;
				case 9 : 
					instructionJ(cpu, instructionEnCours, 0);
					break;
				case 10 : 
					instructionJAL(cpu, instructionEnCours, 0);
					break;
				case 11 : 
					instructionJR(cpu, instructionEnCours, 0);
					break;
				case 12 :
					instructionLUI(cpu, instructionEnCours, 0);
					break;
				case 13 :
					instructionLW(cpu, instructionEnCours, 0);
					break;
				case 14 :
					instructionMFHI(cpu, instructionEnCours, 0);
					break;
				case 15 : 
					instructionMFLO(cpu, instructionEnCours, 0);
					break;
				case 16 : 
					instructionMULT(cpu, instructionEnCours, 0);
					break;
				case 17 : 
					instructionNOP(cpu, instructionEnCours, 0);
					break;
				case 18 : 
					instructionOR(cpu, instructionEnCours, 0);
					break;
				case 19 : 
					instructionROTR(cpu, instructionEnCours, 0);
					break;
				case 20 : 
					instructionSLL(cpu, instructionEnCours, 0);
					break;
				case 21 : 
					instructionSLT(cpu, instructionEnCours, 0);
					break;
				case 22 : 
					instructionSRL(cpu, instructionEnCours, 0);
					break;
				case 23 : 
					instructionSUB(cpu, instructionEnCours, 0);
					break;
				case 24 : 
					instructionSW(cpu, instructionEnCours, 0);
					break;
				case 25 : 
					instructionXOR(cpu, instructionEnCours, 0);
					break;
				default:
					break;
			}
			cpu_dump(cpu);
		}
	}
}

void batch_mode(CPU *cpu, config *config, char *fichierR, char *fichierS1, char *fichierS2){
	int erreur = 0;
	long i = 0, n;
	char chaine[30];
	char espace[2];
	instruction tabInstructions[100];
	FILE *fr, *fs1, *fs2;
	registers_init(cpu);
	memory_init(cpu);
	config->verbose = 1;
	if(config->step){
		if(((fr = fopen(fichierR, "r")) == NULL)){
			printf("Erreur d'ouverture du fichier\n");
			erreur = 1;
		}
	}
	if(!config->step){
		if(((fr = fopen(fichierR, "r")) == NULL) || ((fs1 = fopen(fichierS1, "w")) == NULL) || ((fs2 = fopen(fichierS2, "w")) == NULL)){
			printf("Erreur d'ouverture du/des fichier(s)\n");
			erreur = 1;
		}
	}
	if(!erreur){
		printf("\n----Instruction Decode----\n");
		while(!feof(fr)){
			tabInstructions[i] = decode_instruction(1, fr);
			if(tabInstructions[i].opcode > 0 && tabInstructions[i].opcode <= 25){
				i++;
			}
		}
		printf("\n---------Execute---------\n");
		n = i;
		if(!config->step){
			for(i = 0; i < n; i++){
				fputs(tabInstructions[i].instrHex, fs1);
				fputc(13, fs1);
			}
		}
		i = 0;
		while(i < n){
			switch(tabInstructions[i].opcode){
				case 1 :
					instructionADD(cpu, tabInstructions[i], 1);
					break;
				case 2 :
					instructionADDI(cpu, tabInstructions[i], 1);
					break;
				case 3 :
					instructionAND(cpu, tabInstructions[i], 1);
					break;
				case 4 : 
					instructionBEQ(cpu, tabInstructions[i], 1);
					break;
				case 5 : 
					instructionBGTZ(cpu, tabInstructions[i], 1);
					break;
				case 6 : 
					instructionBLEZ(cpu, tabInstructions[i], 1);
					break;
				case 7 : 
					instructionBNE(cpu, tabInstructions[i], 1);
					break;
				case 8 : 
					instructionDIV(cpu, tabInstructions[i], 1);
					break;
				case 9 : 
					instructionJ(cpu, tabInstructions[i], 1);
					break;
				case 10 : 
					instructionJAL(cpu, tabInstructions[i], 1);
					break;
				case 11 : 
					instructionJR(cpu, tabInstructions[i], 1);
					break;
				case 12 :
					instructionLUI(cpu, tabInstructions[i], 1);
					break;
				case 13 :
					instructionLW(cpu, tabInstructions[i], 1);
					break;
				case 14 :
					instructionMFHI(cpu, tabInstructions[i], 1);
					break;
				case 15 : 
					instructionMFLO(cpu, tabInstructions[i], 1);
					break;
				case 16 : 
					instructionMULT(cpu, tabInstructions[i], 1);
					break;
				case 17 : 
					instructionNOP(cpu, tabInstructions[i], 1);
					break;
				case 18 : 
					instructionOR(cpu, tabInstructions[i], 1);
					break;
				case 19 : 
					instructionROTR(cpu, tabInstructions[i], 1);
					break;
				case 20 : 
					instructionSLL(cpu, tabInstructions[i], 1);
					break;
				case 21 : 
					instructionSLT(cpu, tabInstructions[i], 1);
					break;
				case 22 : 
					instructionSRL(cpu, tabInstructions[i], 1);
					break;
				case 23 : 
					instructionSUB(cpu, tabInstructions[i], 1);
					break;
				case 24 : 
					instructionSW(cpu, tabInstructions[i], 1);
					break;
				case 25 : 
					instructionXOR(cpu, tabInstructions[i], 1);
					break;
				default:
					break;
			}
			if(config->step){
				cpu_dump(cpu);
				fgets(espace, sizeof(espace), stdin);
			}
			i = register_read(cpu, REG_PC);
		}
		if(!config->step){
			cpu_dump(cpu);
			for(i = 0; i < 34; i++){
				if(i <= 31){
					sprintf(chaine, "$%ld : %ld", i, register_read(cpu, (int) i));
					fputs(chaine, fs2);
					fputc(13, fs2);
				}
				else if(i == REG_HI){
					sprintf(chaine, "HI : %ld", register_read(cpu, (int) i));
					fputs(chaine, fs2);
					fputc(13, fs2);
				}
				else{
					sprintf(chaine, "LO : %ld", register_read(cpu, (int) i));
					fputs(chaine, fs2);
					fputc(13, fs2);
				}
			}
		}
	}
}

int main(int argc, char *argv[]){
	CPU cpu;
	config config = {0};

	if(argc == 1){
		interactive_mode(&cpu, &config);
	}
	else if(argc == 3 && !strcmp(argv[2], "-pas")){
		config.step = 1;
		batch_mode(&cpu, &config, argv[1], NULL, NULL);
	}
	else if(argc == 4){
		batch_mode(&cpu, &config, argv[1], argv[2], argv[3]);
	}
	return(0);
}