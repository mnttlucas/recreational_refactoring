#include <stdint.h>
#include <stdio.h>

#include "memory.h"

void memory_init(CPU *cpu)
{
	for(int i = 0; i < MEMORY_SIZE; i++)
		cpu->memory[i] = 0;
}

int32_t memory_read(CPU *cpu, int id)
{
	int32_t value = 0;

	if(id >= 0 && id < MEMORY_SIZE)
		value = cpu->memory[id];
	else
		fprintf(stderr, "[!] memory_read() : invalid memory address @ %d\n", id);
	
	return(value);
}

void memory_write(CPU *cpu, int id, int32_t value)
{
	if(id >= 0 && id < MEMORY_SIZE)
		cpu->memory[id] = value;
	else
		fprintf(stderr, "[!] memory_write() : invalid memory address @ %d\n", id);
}