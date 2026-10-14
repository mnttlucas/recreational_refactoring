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

uint8_t register_string_to_int(char *reg)
{
	int reg_value;
	size_t reg_length = strlen(reg);
	uint8_t reg_1 = (uint8_t) (reg[1] - '0'), reg_2 = 0, reg_int = INVALID_REGISTER, reg_is_only_numbers = 1;

	for(size_t i = 0; i < reg_length; i++)
	{
		if(isalpha(reg[i]))
		{
			reg_is_only_numbers = 0;
			break;
		}
	}
	
	if(reg_is_only_numbers)
	{
		reg_value = atoi(reg);
		if(reg_value >= 0 && reg_value < REGISTER_COUNT) reg_int = (uint8_t) reg_value;
		else reg_int = INVALID_REGISTER;
	}
	else
	{
		if(!mystrcasecmp(reg, "zero")) reg_int = REG_ZERO;
		else if(!mystrcasecmp(reg, "at"))	reg_int = REG_AT;
		else if(!mystrcasecmp(reg, "gp"))	reg_int = REG_GP;
		else if(!mystrcasecmp(reg, "sp"))	reg_int = REG_SP;
		else if(!mystrcasecmp(reg, "fp"))	reg_int = REG_FP;
		else if(!mystrcasecmp(reg, "ra"))	reg_int = REG_RA;
		else if(reg[0] == 'a' || reg[0] == 'A')
		{
			if('0' <= reg[1] && reg[1] <= '3') reg_int = REG_A0 + reg_1;
			else reg_int = INVALID_REGISTER;
		}
		else if(reg[0] == 'f' || reg[0] == 'F')
		{
			if('0' <= reg[1] && reg[1] <= '9')
			{
				if('0' <= reg[2] && reg[2] <= '9' && !reg[3])
				{
					reg_1 *= 10;
					reg_2 = (uint8_t) (reg[2] - '0');
					reg_int = reg_1 + reg_2;
				}
				else if(!reg[2]) reg_int = reg_1;
				else reg_int = INVALID_REGISTER;
			}
			else reg_int = INVALID_REGISTER;
		}
		else if(reg[0] == 'k' || reg[0] == 'K')
		{
			if(reg[1] == '0' || reg[1] == '1') reg_int = REG_K0 + reg_1;
			else reg_int = INVALID_REGISTER;
		}
		else if(reg[0] == 's' || reg[0] == 'S')
		{
			if('0' <= reg[1] && reg[1] <= '7') reg_int = REG_S0 + reg_1;
			else reg_int = INVALID_REGISTER;
		}
		else if(reg[0] == 't' || reg[0] == 'T')
		{
			if('0' <= reg[1] && reg[1] <= '7') reg_int = REG_T0 + reg_1;
			else if(reg[1] == '8' || reg[1] == '9') reg_int = (uint8_t) (REG_T8 + (reg_1 - 8));
			else reg_int = INVALID_REGISTER;
		}
		else if(reg[0] == 'v' || reg[0] == 'V')
		{
			if(reg[1] == '0' || reg[1] == '1') reg_int = REG_V0 + reg_1;
			else reg_int = INVALID_REGISTER;
		}
		else fprintf(stderr, "[!] Unknown register : %c not recognized as an alias\n", reg[0]);
	}

	if(reg_int >= REGISTER_COUNT)
	{
		fprintf(stderr, "[!] Unknown register : %d not a valid index\n", reg_int);
		reg_int = INVALID_REGISTER;
	}

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