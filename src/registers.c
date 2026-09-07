#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "registers.h"

void registers_init(CPU *cpu)
{
	int i;
	for(i = 0; i < REGISTER_COUNT; i++)
	{
		cpu->registers[i] = 0;
	}
}

long register_read(CPU *cpu, int id)
{
	long value;
	if(0 <= id && id < REGISTER_COUNT)
	{
		value = cpu->registers[id];
	}
	return(value);
}

void register_write(CPU *cpu, int id, long value)
{
	if(0 < id && id < REGISTER_COUNT)
	{
		cpu->registers[id] = value;
	}
}