#include <inttypes.h>
#include <stdio.h>

#include "display.h"
#include "mips_memory.h"
#include "mips_registers.h"

typedef void (*dump_cell)(CPU *cpu, int index);

static void print_gpr(CPU *cpu, int index)
{
	printf("$%02d : %-10d ", index, (int32_t) gpr_read_32(cpu, index));
}

static void print_fpr(CPU *cpu, int index)
{
	printf("$%02d : 0x%-16" PRIx64 " ", index, fpr_read_64(cpu, index));
}

static void print_memory_word(CPU *cpu, int index)
{
	uint32_t address = (uint32_t) index * WORD_SIZE;
	printf("@0000 %04" PRIx32 " : 0x%08" PRIx32 "       ", address, memory_read_32(cpu, address));
}

static void dump_grid(CPU *cpu, int count, dump_cell print_cell)
{
	for(int i = 0; i < count; i++)
	{
		print_cell(cpu, i);
		if((i + 1) % DUMP_LINE_SIZE == 0 || i == count - 1) printf("\n");
	}
}

void clear_output(void)
{
	printf("\033[H\033[2J\033[3J");
}

void cpu_dump(CPU *cpu)
{
	int words_to_show = (MEMORY_SIZE / WORD_SIZE < DUMP_MEMORY_WORDS) ? MEMORY_SIZE / WORD_SIZE : DUMP_MEMORY_WORDS;

	printf("\n----------------------- GPR' status ----------------------\n");
	dump_grid(cpu, REGISTER_COUNT, print_gpr);
	printf("\n------------------------------------ FPR' status -----------------------------------\n");
	dump_grid(cpu, REGISTER_COUNT, print_fpr);
	printf("\n------------------------------------------------- Memory status -------------------------------------------------\n");
	dump_grid(cpu, words_to_show, print_memory_word);

	if(words_to_show < MEMORY_SIZE / WORD_SIZE) printf("\n\t\t\t\t\t[...] (%d other words not displayed)\n", MEMORY_SIZE / WORD_SIZE - words_to_show);
}

void log_instruction(instruction *instr)
{
	if(!instr->exit) printf("%s", instr->to_string);
}

void log_usage(char *prog_name)
{
fprintf(stderr, "\n[!] Usage:\n \
- %s for interactive mode\n \
- %s -step {pathfile instructions file} for batch mode (step by step)\n \
- %s {pathfile instructions file} {pathfile hex instructions output} {pathfile registers output} for batch mode\n",
prog_name, prog_name, prog_name);
}

void wait_for_enter(void)
{
	int c;

	printf("\n[#] Press Enter to execute the next instruction");

	do
	{
		c = getchar();
	}
	while(c != '\n' && c != EOF);
}
