#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "cpu.h"
#include "decode.h"
#include "execute.h"
#include "instruction.h"
#include "mips_registers.h"
#include "modes.h"
#include "utils.h"

int load_program(const char *path, Program *prog)
{
	FILE *in = open_file(path, "r");
	instruction cur;
	uint32_t error_cnt = 0, line_no = 0;

	if(!in) 
	{
		fprintf(stderr, "[!] Cannot open file %s : %s\n", path, strerror(errno));
		return(0);
	}

	printf("\n--- Instruction decode ---\n");
	while(1)
	{
		cur = decode_instruction(BATCH, in);
		if(cur.exit) break;
		if(cur.error)
		{
			fprintf(stderr, "[!] batch_mode : line %u invalid\n", line_no);
			error_cnt++;
		}
		else if(cur.opcode > OPCODE_MIN && cur.opcode < OPCODE_MAX)
		{
			if(prog->count >= prog->capacity && !program_grow(prog))
			{
				fclose(in);
				return(0);
			}
			prog->items[prog->count] = cur;
			prog->count++;
		}
	}
	fclose(in);

	if(error_cnt)
	{
		fprintf(stderr, "[!] batch_mode : %u invalid line(s), program not executed\n", error_cnt);
		return(0);
	}

	return(1);
}

FILE *open_file(const char *path, const char *mode)
{
	FILE *file = fopen(path, mode);

	if(!file) fprintf(stderr, "[!] Cannot open file %s : %s\n", path, strerror(errno));

	return(file);
}

int program_grow(Program *prog)
{
	size_t new_capacity = prog->capacity ? prog->capacity * 2 : INITIAL_CAPACITY;
	instruction *new_items = realloc(prog->items, new_capacity * sizeof(instruction));

	if(!new_items)
	{
		fprintf(stderr, "[!] batch_mode : realloc() error\n");
		return(0);
	}

	prog->capacity = new_capacity;
	prog->items = new_items;

	return(1);
}

void run_program(CPU *cpu, Config *cfg, const Program *prog)
{
	uint32_t pc_index = 0;

	printf("\n-- Instruction  execute --\n");
	while(pc_index < prog->count)
	{
		if(cfg->step) clear_output();
		execute_instruction(cpu, cfg, prog->items[pc_index]);
		if(cfg->step) cpu_dump(cpu, cfg);
		pc_index = cpu->PC / 4;
	}
}

void write_hex(FILE *out, const Program *prog)
{
	for(uint32_t instr_id = 0; instr_id < prog->count; instr_id++) fprintf(out, "%s\n", prog->items[instr_id].instr_hex);
}

void write_regs(FILE *out, CPU *cpu)
{
	for(uint8_t reg_id = 0; reg_id < REGISTER_COUNT; reg_id++) fprintf(out, "$%d : %d\n", reg_id, (int32_t) gpr_read_32(cpu, reg_id));
}

void interactive_mode(CPU *cpu, Config *cfg)
{
	instruction cur = {0};

	while(!cur.exit)
	{
		printf("[#] Enter your instruction :\n");
		cur = decode_instruction(INTERACTIVE, NULL);
		if(!cur.exit && cur.opcode > OPCODE_MIN && cur.opcode < OPCODE_MAX)
		{
			execute_instruction(cpu, cfg, cur);
			cpu_dump(cpu, cfg);
		}
	}
}

void batch_mode(CPU *cpu, Config *cfg, char *path_in, char *path_out_hex, char *path_out_regs)
{
	FILE *out_hex = NULL, *out_regs = NULL;
	Program prog = {0};

	if(load_program(path_in, &prog))
	{
		if(!cfg->step)
		{
			out_hex = open_file(path_out_hex, "w");
			if(out_hex) out_regs = open_file(path_out_regs, "w");
		}
		if(cfg->step || out_regs)
		{
			run_program(cpu, cfg, &prog);
			if(!cfg->step)
			{
				write_hex(out_hex, &prog);
				cpu_dump(cpu, cfg);
				write_regs(out_regs, cpu);
			}
		}
	}

	if(out_hex) fclose(out_hex);
	if(out_regs) fclose(out_regs);
	free(prog.items);
}