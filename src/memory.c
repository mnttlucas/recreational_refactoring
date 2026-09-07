#include "memory.h"

void memory_init(CPU *cpu)
{
	for(int i = 0; i < MEMORY_SIZE; i++)
		cpu->memory[i] = 0;
}

int memory_read(CPU *cpu, int id)
{
	int value;

	if(id >= 0 && id < MEMORY_SIZE)
		value = cpu->memory[id];
	
	return(value);
}

void memory_write(CPU *cpu, int id, int value)
{
	if(id >= 0 && id < MEMORY_SIZE)
		cpu->memory[id] = value;
}