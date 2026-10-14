#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mips_memory.h"
#include "mips_registers.h"
#include "utils.h"

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

static int to_lower_ascii(int c)
{
	return((c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c);
}

void clear_output()
{
	printf("\033[2J\033[H\n\n");
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

	if(words_to_show < MEMORY_SIZE / WORD_SIZE) printf("\t\t\t[...] (%d other words not displayed)\n", MEMORY_SIZE / WORD_SIZE - words_to_show);
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

int mystrcasecmp(const char *a, const char *b)
{
	while(*a && to_lower_ascii((unsigned char) *a) == to_lower_ascii((unsigned char) *b))
	{
		a++;
		b++;
	}

	return(to_lower_ascii((unsigned char) *a) - to_lower_ascii((unsigned char) *b));
}

char *read_full_line(FILE *in)
{
	size_t capacity = 256, length = 0;
	char *line = malloc(capacity);

	if(!line) return(NULL);

	while(fgets(line + length, (int) (capacity - length), in))
	{
		length += strlen(line + length);

		if(length > 0 && line[length - 1] == '\n') return(line);
		if(feof(in)) return(line);

		capacity *= 2;
		char *tmp = realloc(line, capacity);
		if(!tmp)
		{
			free(line);
			return(NULL);
		}
		line = tmp;
	}
	
	if(length == 0)
	{
		free(line);
		return(NULL);
	}

	return(line);
}

typedef struct
{
	const char *name;
	uint8_t index;
} register_alias;

/* Register names (without the leading '$'), see MIPS32 o32 ABI */
static const register_alias register_aliases[] = {
	{"zero", REG_ZERO}, {"at", REG_AT},
	{"v0", REG_V0}, {"v1", REG_V1},
	{"a0", REG_A0}, {"a1", REG_A1}, {"a2", REG_A2}, {"a3", REG_A3},
	{"t0", REG_T0}, {"t1", REG_T1}, {"t2", REG_T2}, {"t3", REG_T3},
	{"t4", REG_T4}, {"t5", REG_T5}, {"t6", REG_T6}, {"t7", REG_T7},
	{"s0", REG_S0}, {"s1", REG_S1}, {"s2", REG_S2}, {"s3", REG_S3},
	{"s4", REG_S4}, {"s5", REG_S5}, {"s6", REG_S6}, {"s7", REG_S7},
	{"t8", REG_T8}, {"t9", REG_T9},
	{"k0", REG_K0}, {"k1", REG_K1},
	{"gp", REG_GP}, {"sp", REG_SP}, {"fp", REG_FP}, {"ra", REG_RA}};

/* Parses a register number made of one or two digits ("5", "05", "31").
   Returns INVALID_REGISTER if there is anything else or if it is out of range. */
static uint8_t parse_register_number(const char *str)
{
	size_t length = strlen(str);
	int value = 0;

	if(length == 0 || length > 2) return(INVALID_REGISTER);
	for(size_t i = 0; i < length; i++)
	{
		if(!isdigit((unsigned char) str[i])) return(INVALID_REGISTER);
		value = value * 10 + (str[i] - '0');
	}

	return(value < REGISTER_COUNT ? (uint8_t) value : INVALID_REGISTER);
}

/* Accepts "$5", "$t0", "$T0", "$f12" without the '$' : number, alias or FPR name.
   Returns INVALID_REGISTER (and prints the reason) when the name is not a register. */
uint8_t register_string_to_int(const char *reg)
{
	uint8_t reg_int = parse_register_number(reg);

	if(reg_int == INVALID_REGISTER && (reg[0] == 'f' || reg[0] == 'F')) reg_int = parse_register_number(reg + 1);

	for(size_t i = 0; reg_int == INVALID_REGISTER && i < ARR_SIZE(register_aliases); i++)
		if(!mystrcasecmp(reg, register_aliases[i].name)) reg_int = register_aliases[i].index;

	if(reg_int == INVALID_REGISTER) fprintf(stderr, "[!] Unknown register : '%s'\n", reg);

	return(reg_int);
}

int32_t sign_extend(int32_t value, uint8_t bits)
{
	uint32_t raw = (uint32_t) value << (32 - bits);
	return((int32_t) raw >> (32 - bits));
}

void wait_for_enter()
{
	int c;

	printf("\n[#] Press Enter to execute the next instruction");

	do
	{
		c = getchar();
	}
	while(c != '\n' && c != EOF);
}