#include <stdio.h>

#include "registers.h"

void registers_init(CPU *cpu)
{
	for(int i = 0; i < REGISTER_COUNT; i++)
		cpu->registers[i] = 0;
}

long register_read(CPU *cpu, int id)
{
	long value = 0;

	if(id >= 0 && id < REGISTER_COUNT)
		value = cpu->registers[id];
	else
		fprintf(stderr, "[!] register_read() : invalid register %d\n", id);
	
	return(value);
}

void register_write(CPU *cpu, int id, long value)
{
	if(id > 0 && id < REGISTER_COUNT)
		cpu->registers[id] = value;
	else
		fprintf(stderr, "[!] register_write() : invalid register %d\n", id);
}