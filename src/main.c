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
	int i = 0, n;
	unsigned long capacity = 64;
	instruction *instructions_arr = malloc(capacity * sizeof(instruction));
	FILE *in, *out_hex, *out_regs;

	if(!instructions_arr)
	{
		printf("\n[!] batch_mode : malloc() error\n");
		return;
	}

	if(cfg->step)
	{
		if(!(in = fopen(path_in, "r")))
		{
			printf("\n[!] batch_mode : fopen() error\n");
			return;
		}
	}
	else
	{
		if(!(in = fopen(path_in, "r")))
		{
			printf("\n[!] batch_mode : fopen() error\n");
			return;
		}
		if(!(out_hex = fopen(path_out_hex, "w")))
		{
			printf("\n[!] batch_mode : fopen() error\n");
			fclose(in);
			return;
		}
		if(!(out_regs = fopen(path_out_regs, "w")))
		{
			printf("\n[!] batch_mode : fopen() error\n");
			fclose(in);
			fclose(out_regs);
			return;
		}
	}

	printf("\n--- Instruction decode ---\n");
	while(1)
	{
		if(i >= (int) capacity)
		{
			capacity *= 2;
			instruction *tmp = realloc(instructions_arr, capacity * sizeof(instruction));
			if(tmp)
				instructions_arr = tmp;
			else
			{
				printf("\n[!] batch_mode : realloc() error\n");
				return;
			}
		}
		instructions_arr[i] = decode_instruction(1, in);
		if(instructions_arr[i].exit)
			break;
		if(instructions_arr[i].opcode > OPCODE_MIN && instructions_arr[i].opcode < OPCODE_MAX)
			i++;
	}

	printf("\n-- Instruction  execute --\n");
	n = i;
	if(!cfg->step)
		for(i = 0; i < n; i++)
			fprintf(out_hex, "%s\n", instructions_arr[i].instr_hex);

	i = 0;
	while(i < n)
	{
		if(cfg->step)
			clear_output();
		execute_instruction(cpu, cfg, instructions_arr[i]);
		if(cfg->step)
			cpu_dump(cpu, cfg);
		i = register_read(cpu, REG_PC);
	}

	if(!cfg->step)
	{
		cpu_dump(cpu, cfg);
		for(i = 0; i < 32; i++)
			fprintf(out_regs, "$%d : %d\n", i, register_read(cpu, i));
		fprintf(out_regs, "HI : %d\n", register_read(cpu, REG_HI));
		fprintf(out_regs, "LO : %d\n", register_read(cpu, REG_LO));
	}

	fclose(in);
	if(path_out_hex && path_out_regs)
	{
		fclose(out_hex);
		fclose(out_regs);
	}
	free(instructions_arr);
}

int main(int argc, char *argv[])
{
	config cfg = {0};
	CPU cpu;

	memory_init(&cpu);
	registers_init(&cpu);

	if(argc == 1)
		interactive_mode(&cpu, &cfg);
	else
	{
		cfg.verbose = 1;
		if(argc == 3 && !strcmp(argv[2], "-step"))
		{
			cfg.step = 1;
			batch_mode(&cpu, &cfg, argv[1], NULL, NULL);
		}
		else if(argc == 4)
			batch_mode(&cpu, &cfg, argv[1], argv[2], argv[3]);
		else
			log_usage(argv[0]);

	}
	return(0);
}