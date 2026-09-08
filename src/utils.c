#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "registers.h"
#include "utils.h"

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

void cpu_dump(CPU *cpu, config *cfg)
{
	char hex_char[4] = {'0', '4', '8', 'C'}, wait[2];

	printf("\033[2J\033[H\n-------------------- Registers' status -------------------\n");
	for(int i = 0; i < 8; i++)
	{
		for(int j = 0; j < 4; j++)
			printf("$%s%d : %-10ld ", 4 * i + j <= 9 ? "0" : "", 4 * i + j, register_read(cpu, 4 * i + j));
		printf("\n");
	}
	printf("                 HI  : %-10ld LO  : %-10ld\n", register_read(cpu, REG_HI), register_read(cpu, REG_LO));

	printf("\n------------------------------------ Memory status -----------------------------------\n");
	for(int i = 0; i < 5; i++)
	{
		for(int j = 0; j < 4; j++)
			printf("@0000 00%d%c : %-10d ", i, hex_char[j], memory_read(cpu, 4 * i + j));
		printf("\n");
	}
	printf("\n");

	if(cfg->step)
		fgets(wait, sizeof(wait), stdin);
}

void increment_pc(CPU *cpu)
{
	register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
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
	int reg_int, reg_is_only_numbers = 1;
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
			reg_int = 0;
		else if(!strcmp(reg, "at"))
			reg_int = 1;
		else if(!strcmp(reg, "gp"))
			reg_int = 28;
		else if(!strcmp(reg, "sp"))
			reg_int = 29;
		else if(!strcmp(reg, "fp"))
			reg_int = 30;
		else if(!strcmp(reg, "ra"))
			reg_int = 31;
		else if(reg[0] == 'v')
			reg_int = 2 + reg[1] - '0';
		else if(reg[0] == 'a')
			reg_int = 4 + reg[1] - '0';
		else if(reg[0] == 's')
			reg_int = 16 + reg[1] - '0';
		else if(reg[0] == 'k')
			reg_int = 26 + reg[1] - '0';
		else if(reg[0] == 't')
		{
			if(reg[1] - '0' <= 7)
				reg_int = 8 + reg[1] - '0';
			else
				reg_int = 24 + reg[1] - '0';
		}
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