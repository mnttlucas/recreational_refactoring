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

void close_files(FILE *in, FILE *out_hex, FILE *out_regs, char *path_out_hex, char *path_out_regs)
{
	fclose(in);
	if(path_out_hex && path_out_regs)
	{
		fclose(out_hex);
		fclose(out_regs);
	}
}

void interactive_mode(CPU *cpu, config *cfg)
{
	instruction current_instr = {0};

	while(!current_instr.exit)
	{
		printf("[#] Enter your instruction :\n");
		current_instr = decode_instruction(0, NULL);
		if(!current_instr.exit)
		{
			execute_instruction(cpu, cfg, current_instr);
			cpu_dump(cpu, cfg);
		}
	}
}

void batch_mode(CPU *cpu, config *cfg, char *path_in, char *path_out_hex, char *path_out_regs)
{
	long i = 0, n;
	instruction instructions_arr[100];
	FILE *in, *out_hex, *out_regs;

	if(cfg->step)
	{
		if(((in = fopen(path_in, "r")) == NULL))
		{
			printf("[!] batch_mode : fopen() error\n");
			return;
		}
	}
	else
	{
		if(((in = fopen(path_in, "r")) == NULL) || 
			((out_hex = fopen(path_out_hex, "w")) == NULL) ||
			((out_regs = fopen(path_out_regs, "w")) == NULL))
		{
			printf("[!] batch_mode : fopen() error\n");
			return;
		}
	}

	printf("\n--- Instruction decode ---\n");
	while(1)
	{
		instructions_arr[i] = decode_instruction(1, in);
		if(instructions_arr[i].exit)
		{
			close_files(in, out_hex, out_regs, path_out_hex, path_out_regs);
			break;
		}
		if(instructions_arr[i].opcode > 0 && instructions_arr[i].opcode <= 25)
			i++;
	}

	printf("\n-- Instruction  execute --\n");
	n = i;
	if(!cfg->step)
		for(i = 0; i < n; i++)
			fprintf(out_hex, "%s\n", instructions_arr[i].instrHex);

	i = 0;
	while(i < n)
	{
		execute_instruction(cpu, cfg, instructions_arr[i]);
		if(cfg->step)
			cpu_dump(cpu, cfg);
		i = register_read(cpu, REG_PC);
	}

	if(!cfg->step)
	{
		cpu_dump(cpu, cfg);
		for(i = 0; i < 32; i++)
			fprintf(out_regs, "$%ld : %ld\n", i, register_read(cpu, (int) i));
		fprintf(out_regs, "HI : %ld\n", register_read(cpu, REG_HI));
		fprintf(out_regs, "LO : %ld\n", register_read(cpu, REG_LO));
	}

	close_files(in, out_hex, out_regs, path_out_hex, path_out_regs);
}

int main(int argc, char *argv[])
{
	CPU cpu;
	config cfg = {0};

	memory_init(&cpu);
	registers_init(&cpu);

	if(argc == 1)
	{
		cfg.verbose = 0;
		interactive_mode(&cpu, &cfg);
	}
	else{
		cfg.verbose = 1;
		if(argc == 3 && !strcmp(argv[2], "-step"))
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
fprintf(stderr, "[!] Usage:\n \
- %s for interactive mode\n \
- %s -step {pathfile instructions file} for batch mode (step by step)\n \
- %s {pathfile instructions file} {pathfile hex instructions output} {pathfile registers output} for batch mode\n",
argv[0], argv[0], argv[0]);
		}
	}
	return(0);
}