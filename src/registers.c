#include <stdint.h>
#include <stdio.h>

#include "registers.h"

void registers_init(CPU *cpu)
{
	for(int i = 0; i < REGISTER_COUNT; i++)
		cpu->registers[i] = 0;
}

int32_t register_read(CPU *cpu, int id)
{
	int32_t value = 0;

	if(id >= 0 && id < REGISTER_COUNT)
		value = cpu->registers[id];
	else
		fprintf(stderr, "\n[!] register_read() : invalid register %d\n", id);
	
	return(value);
}

void register_write(CPU *cpu, int id, int32_t value)
{
	if(id > 0 && id < REGISTER_COUNT)
		cpu->registers[id] = value;
	else
		fprintf(stderr, "\n[!] register_write() : invalid register %d\n", id);
}