#include <stdio.h>
#include <string.h>

#include "config.h"
#include "cpu.h"
#include "decode.h"
#include "execute.h"
#include "instruction.h"
#include "memory.h"
#include "registers.h"
#include "utils.h"

void interactive_mode(CPU *cpu, config *cfg)
{
	instruction current_instr;
	current_instr.exit = 0;

	registers_init(cpu);
	memory_init(cpu);

	while(current_instr.exit == 0)
	{
		printf("[#] Enter your instruction :\n");
		current_instr = decode_instruction(0, NULL);
		if(current_instr.exit == 0)
		{
			execute_instruction(cpu, cfg, current_instr);
			cpu_dump(cpu, cfg);
		}
	}
}

void batch_mode(CPU *cpu, config *cfg, char *fichierR, char *fichierS1, char *fichierS2){
	int error = 0;
	long i = 0, n;
	char chaine[30];
	instruction tabInstructions[100];
	FILE *fr, *fs1, *fs2;

	registers_init(cpu);
	memory_init(cpu);

	if(cfg->step)
		if(((fr = fopen(fichierR, "r")) == NULL))
		{
			printf("[!] batch_mode : fopen() error\n");
			error = 1;
		}
	if(!cfg->step)
		if(((fr = fopen(fichierR, "r")) == NULL) || ((fs1 = fopen(fichierS1, "w")) == NULL) || ((fs2 = fopen(fichierS2, "w")) == NULL)){
			printf("[!] batch_mode : fopen() error\n");
			error = 1;
		}
	if(!error){
		printf("\n--- Instruction decode ---\n");
		while(!feof(fr)){
			tabInstructions[i] = decode_instruction(1, fr);
			if(tabInstructions[i].opcode > 0 && tabInstructions[i].opcode <= 25){
				i++;
			}
		}
		printf("\n-- Instruction  execute --\n");
		n = i;
		if(!cfg->step){
			for(i = 0; i < n; i++){
				fputs(tabInstructions[i].instrHex, fs1);
				fputc(13, fs1);
			}
		}
		i = 0;
		while(i < n){
			execute_instruction(cpu, cfg, tabInstructions[i]);
			if(cfg->step)
				cpu_dump(cpu, cfg);
			i = register_read(cpu, REG_PC);
		}
		if(!cfg->step){
			cpu_dump(cpu, cfg);
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

int main(int argc, char *argv[])
{
	CPU cpu;
	config cfg = {0};

	if(argc == 1)
	{
		cfg.verbose = 0;
		interactive_mode(&cpu, &cfg);
	}
	else{
		cfg.verbose = 1;
		if(argc == 3 && !strcmp(argv[2], "-pas"))
		{
			cfg.step = 1;
			batch_mode(&cpu, &cfg, argv[1], NULL, NULL);
		}
		else if(argc == 4)
		{
			batch_mode(&cpu, &cfg, argv[1], argv[2], argv[3]);
		}
		else
		{
		fprintf(stderr, 
"[!] Usage:\n \
- %s pour le mode interactif\n \
- %s -pas {pathfile fichier d'instructions} pour le mode automatique en pas à pas\n \
- %s {pathfile fichier d'instructions} {pathfile sortie décodée} {pathfile sortie registre} pour le mode automatique\n",
argv[0], argv[0], argv[0]);
		}
	}
	return(0);
}