#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "registers.h"
#include "utils.h"

#define DUMP_LINE_SIZE 4

void bin_arr_to_hex_arr(int *bin_arr, char *hex_arr)
{
	int bit_value, hex_value;

	for(int i = 0; i < 8; i++)
	{
		bit_value = 8;
		hex_value = 0;
		for(int j = 0; j < 4; j++)
		{
			hex_value += bit_value * bin_arr[i * 4 + j];
			bit_value /= 2;
		}
		if(hex_value <= 9)
			hex_arr[i] = (char) ('0' + hex_value);
		else{
			switch(hex_value)
			{
				case 10 : hex_arr[i] = 'a'; break;
				case 11 : hex_arr[i] = 'b'; break;
				case 12 : hex_arr[i] = 'c'; break;
				case 13 : hex_arr[i] = 'd'; break;
				case 14 : hex_arr[i] = 'e'; break;
				case 15 : hex_arr[i] = 'f'; break;
			}
		}
	}

	hex_arr[8] = '\0';
}

void bin_twos_complement(int start, int end, int *arr)
{
	int found_one = 0;

	if(start < 0 || start > end)
		return;

	for(int i = end; i >= start; i--)
	{
		if(found_one)
			arr[i] ^= 1;
		else if(arr[i] == 1)
			found_one = 1;
	}
}

void bin_zero(int *arr)
{
	for(int i = 0; i < 32; i++)
		arr[i] = 0;
}

void clear_output()
{
	printf("\033[2J\033[H");
}

void cpu_dump(CPU *cpu, config *cfg)
{
	char hex_char[4] = {'0', '4', '8', 'C'};

	printf("\n-------------------- Registers' status -------------------\n");
	for(int i = 0; i <= (REGISTER_COUNT - 3) / DUMP_LINE_SIZE; i++)
	{
		for(int j = 0; (i * DUMP_LINE_SIZE + j) < (REGISTER_COUNT - 3) && j < DUMP_LINE_SIZE; j++)
			printf("$%s%d : %-10d ", 4 * i + j <= 9 ? "0" : "", 4 * i + j, register_read(cpu, 4 * i + j));
		printf("\n");
	}
	printf("                 HI  : %-10d LO  : %-10d\n", register_read(cpu, REG_HI), register_read(cpu, REG_LO));

	printf("\n------------------------------------ Memory status -----------------------------------\n");
	for(int i = 0; i <= (MEMORY_SIZE - 1) / DUMP_LINE_SIZE; i++)
	{
		for(int j = 0; (i * DUMP_LINE_SIZE + j) < MEMORY_SIZE && j < DUMP_LINE_SIZE; j++)
			printf("@0000 00%d%c : %-10d ", i, hex_char[j], memory_read(cpu, DUMP_LINE_SIZE * i + j));
		printf("\n");
	}
	printf("\n");

	if(cfg->step)
		wait_for_enter();
}

void increment_pc(CPU *cpu)
{
	register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
}

int is_negative(char *str)
{
	return(str[0] == '-');
}

void log_instruction(instruction *instr)
{
	if(!instr->exit)
		printf("%s", instr->toString);
}

void log_usage(char *prog_name)
{
fprintf(stderr, "\n[!] Usage:\n \
- %s for interactive mode\n \
- %s -step {pathfile instructions file} for batch mode (step by step)\n \
- %s {pathfile instructions file} {pathfile hex instructions output} {pathfile registers output} for batch mode\n",
prog_name, prog_name, prog_name);
}

void long_to_bin_arr(int start, int end, long value, int *arr)
{
	long bin = 1;

	for(int i = start; i < end; i++)
		bin *= 2;
	for(int i = start; i <= end; i++)
	{
		if(value >= bin)
		{
			value -= bin;
			arr[i] = 1;
		}
		bin /= 2;
	}
}

int register_string_to_int(char *reg)
{
	int reg_1 = reg[1] - '0', reg_int = 0, reg_is_only_numbers = 1;
	size_t reg_length = strlen(reg);

	for(size_t i = 0; i < reg_length; i++)
	{
		if(isalpha(reg[i]))
		{
			reg_is_only_numbers = 0;
			break;
		}
	}
	
	if(reg_is_only_numbers)
		reg_int = atoi(reg);
	else
	{
		if(!strcmp(reg, "zero"))
			reg_int = REG_ZERO;
		else if(!strcmp(reg, "at"))
			reg_int = REG_AT;
		else if(!strcmp(reg, "gp"))
			reg_int = REG_GP;
		else if(!strcmp(reg, "sp"))
			reg_int = REG_SP;
		else if(!strcmp(reg, "fp"))
			reg_int = REG_FP;
		else if(!strcmp(reg, "ra"))
			reg_int = REG_RA;
		else if(reg[0] == 'v')
		{
			if(reg[1] == '0' || reg[1] == '1')
				reg_int = REG_V0 + reg_1;
		}
		else if(reg[0] == 'a')
		{
			if('0' <= reg[1] && reg[1] <= '3')
				reg_int = REG_A0 + reg_1;
		}
		else if(reg[0] == 's')
		{
			if('0' <= reg[1] && reg[1] <= '7')
				reg_int = REG_S0 + reg_1;
		}
		else if(reg[0] == 'k')
		{
			if(reg[1] == '0' || reg[1] == '1')
			reg_int = REG_K0 + reg_1;
		}
		else if(reg[0] == 't')
		{
			if('0' <= reg[1] && reg[1] <= '7')
				reg_int = REG_T0 + reg_1;
			else if(reg[1] == '8' || reg[1] == '9')
				reg_int = REG_T8 + reg_1;
		}
		else
			fprintf(stderr, "[!] Unknown register, returned 0 to ignore\n");
	}

	return(reg_int);
}

void remove_sign(char *str)
{
	int i = 0;

	while(str[i + 1] != '\0')
	{
		str[i] = str[i + 1];
		i++;
	}

	str[i] = '\0';
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