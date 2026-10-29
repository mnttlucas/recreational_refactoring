#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "cpu.h"
#include "decode.h"
#include "display.h"
#include "execute.h"
#include "instruction.h"
#include "mips_registers.h"
#include "modes.h"
#include "utils.h"

#define INITIAL_CAPACITY 64

typedef struct
{
	Instruction *items;
	uint32_t count;
	size_t capacity;
} Program;

static FILE *open_file(const char *path, const char *mode)
{
	FILE *file = fopen(path, mode);

	if(!file) fprintf(stderr, "[!] Cannot open file %s : %s\n", path, strerror(errno));

	return(file);
}

static int program_grow(Program *prog)
{
	size_t new_capacity = prog->capacity ? prog->capacity * 2 : INITIAL_CAPACITY;
	Instruction *new_items = realloc(prog->items, new_capacity * sizeof(Instruction));

	if(!new_items)
	{
		fprintf(stderr, "[!] batch_mode : realloc() error\n");
		return(0);
	}

	prog->capacity = new_capacity;
	prog->items = new_items;

	return(1);
}

static int load_program(const char *path, Program *prog, int hex)
{
	FILE *in = open_file(path, "r");
	Instruction cur;
	uint32_t error_cnt = 0, line_no = 0;

	if(!in) return(0);

	printf("\n--- Instruction decode ---\n\n");
	while(1)
	{
		cur = decode_instruction(BATCH, in, hex);
		line_no++;
		if(cur.exit) break;
		if(cur.error != NO_ERR)
		{
			fprintf(stderr, "[!] batch_mode : line %u invalid\n", line_no);
			error_cnt++;
		}
		else if(cur.opcode > OP_MIN && cur.opcode < OP_MAX)
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

/* Returns 1 if an exception stopped the program, 0 if it ran to its end */
static int run_program(Cpu *cpu, Config *cfg, const Program *prog)
{
	uint32_t pc_index = 0;

	printf("\n-- Instruction  execute --\n\n");
	while(pc_index < prog->count && !cpu->exception.raised)
	{
		if(cfg->step) clear_output();
		execute_instruction(cpu, cfg, &prog->items[pc_index]);
		if(cfg->step)
		{
			cpu_dump(cpu, cfg);
			wait_for_enter();
		}
		pc_index = cpu->PC / WORD_SIZE;
	}

	return(cpu->exception.raised);
}

static void write_hex(FILE *out, const Program *prog)
{
	for(uint32_t instr_id = 0; instr_id < prog->count; instr_id++) fprintf(out, "%08" PRIx32 "\n", prog->items[instr_id].encoding);
}

static void write_regs(FILE *out, Cpu *cpu)
{
	for(uint8_t reg_id = 0; reg_id < REGISTER_COUNT; reg_id++) fprintf(out, "$%d : %d\n", reg_id, (int32_t) gpr_read_32(cpu, reg_id));
}

void interactive_mode(Cpu *cpu, Config *cfg)
{
	Instruction cur = {0};

	while(!cur.exit)
	{
		printf("[#] Enter your instruction :\n");
		cur = decode_instruction(INTERACTIVE, NULL, cfg->hex);
		if(!cur.exit && cur.opcode > OP_MIN && cur.opcode < OP_MAX)
		{
			execute_instruction(cpu, cfg, &cur);
			cpu_dump(cpu, cfg);
			/* Each instruction is typed on its own, nothing is stopped : the state is the one before the instruction */
			cpu->exception.raised = 0;
		}
	}
}

int batch_mode(Cpu *cpu, Config *cfg, char *path_in, char *path_out_hex, char *path_out_regs)
{
	FILE *out_hex = NULL, *out_regs = NULL;
	int status = EXIT_FAILURE;
	Program prog = {0};

	if(load_program(path_in, &prog, cfg->hex))
	{
		if(!cfg->step)
		{
			if(path_out_hex) out_hex = open_file(path_out_hex, "w");
			if(!path_out_hex || out_hex) out_regs = open_file(path_out_regs, "w");
		}
		if(cfg->step || out_regs)
		{
			status = run_program(cpu, cfg, &prog) ? EXIT_FAILURE : EXIT_SUCCESS;
			if(!cfg->step)
			{
				if(out_hex) write_hex(out_hex, &prog);
				cpu_dump(cpu, cfg);
				write_regs(out_regs, cpu);
			}
		}
	}

	if(out_hex && fclose(out_hex) != 0) status = EXIT_FAILURE;
	if(out_regs && fclose(out_regs) != 0) status = EXIT_FAILURE;
	free(prog.items);

	return(status);
}
