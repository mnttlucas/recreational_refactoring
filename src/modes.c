#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "cpu.h"
#include "decode.h"
#include "execute.h"
#include "instruction.h"
#include "mips_registers.h"
#include "utils.h"

void interactive_mode(CPU *cpu, Config *cfg)
{
	instruction current_instr = {0};

	while(!current_instr.exit)
	{
		printf("[#] Enter your instruction :\n");
		current_instr = decode_instruction(0, NULL);
		if(!current_instr.exit && current_instr.opcode > OPCODE_MIN && current_instr.opcode < OPCODE_MAX)
		{
			execute_instruction(cpu, cfg, current_instr);
			cpu_dump(cpu, cfg);
		}
	}
}

void batch_mode(CPU *cpu, Config *cfg, char *path_in, char *path_out_hex, char *path_out_regs)
{
	uint32_t error_count = 0, i = 0, line_number = 0, n;
	unsigned long capacity = 64;
	instruction *instructions_arr = malloc(capacity * sizeof(instruction));
	FILE *in, *out_hex, *out_regs;

	if(!instructions_arr)
	{
		printf("\n[!] batch_mode : malloc() error\n");
		free(instructions_arr);
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
			free(instructions_arr);
			return;
		}
		if(!(out_hex = fopen(path_out_hex, "w")))
		{
			printf("\n[!] batch_mode : fopen() error\n");
			fclose(in);
			free(instructions_arr);
			return;
		}
		if(!(out_regs = fopen(path_out_regs, "w")))
		{
			printf("\n[!] batch_mode : fopen() error\n");
			fclose(in);
			fclose(out_hex);
			free(instructions_arr);
			return;
		}
	}

	printf("\n--- Instruction decode ---\n");
	while(1)
	{
		if(i >= capacity)
		{
			capacity *= 2;
			instruction *tmp = realloc(instructions_arr, capacity * sizeof(instruction));
			if(tmp) instructions_arr = tmp;
			else
			{
				printf("\n[!] batch_mode : realloc() error\n");
				fclose(in);
				if(!cfg->step)
				{
					fclose(out_hex);
					fclose(out_regs);
				}
				return;
			}
		}
		instructions_arr[i] = decode_instruction(1, in);
		line_number++;
		if(instructions_arr[i].exit) break;
		if(instructions_arr[i].error)
		{
			fprintf(stderr, "[!] batch_mode : line %u invalid\n", line_number);
			error_count++;
		}
		else if(instructions_arr[i].opcode > OPCODE_MIN && instructions_arr[i].opcode < OPCODE_MAX) i++;
	}

	if(error_count)
	{
		fprintf(stderr, "[!] batch_mode : %u invalid line(s), program not executed\n", error_count);
		fclose(in);
		if(!cfg->step)
		{
			fclose(out_hex);
			fclose(out_regs);
		}
		free(instructions_arr);
		return;
	}

	printf("\n-- Instruction  execute --\n");
	n = i;
	if(!cfg->step)
		for(i = 0; i < n; i++) fprintf(out_hex, "%s\n", instructions_arr[i].instr_hex);

	i = 0;
	while(i < n)
	{
		if(cfg->step) clear_output();
		execute_instruction(cpu, cfg, instructions_arr[i]);
		if(cfg->step) cpu_dump(cpu, cfg);
		i = cpu->PC / 4;
	}

	if(!cfg->step)
	{
		cpu_dump(cpu, cfg);
		for(i = 0; i < REGISTER_COUNT; i++) fprintf(out_regs, "$%d : %d\n", i, gpr_read_32(cpu, (int) i));
	}

	fclose(in);
	if(path_out_hex && path_out_regs)
	{
		fclose(out_hex);
		fclose(out_regs);
	}
	free(instructions_arr);
}
