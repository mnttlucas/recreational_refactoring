#include <stdint.h>
#include <stdio.h>

#include "mips_memory.h"

void memory_init(CPU *cpu)
{
	for(int i = 0; i < MEMORY_SIZE; i++) cpu->memory[i] = 0;
}

int16_t memory_read_16(CPU *cpu, int address)
{
	uint16_t value = 0;

	if(address >= 0 && address + 1 < MEMORY_SIZE)
	{
		value = (uint16_t) cpu->memory[address] << 8;
		value |= (uint16_t) cpu->memory[address + 1];
	}
	else fprintf(stderr, "[!] memory_read_16() : invalid memory address @ %d\n", address);
	
	return((int16_t) value);
}

int32_t memory_read_32(CPU *cpu, int address)
{
	uint32_t value = 0;

	if(address >= 0 && address + 3 < MEMORY_SIZE)
	{
		value = (uint32_t) cpu->memory[address] << 24;
		value |= (uint32_t) cpu->memory[address + 1] << 16;
		value |= (uint32_t) cpu->memory[address + 2] << 8;
		value |= (uint32_t) cpu->memory[address + 3];
	}
	else fprintf(stderr, "[!] memory_read_32() : invalid memory address @ %d\n", address);
	
	return((int32_t) value);
}

int8_t memory_read_8(CPU *cpu, int address)
{
	int8_t value = 0;

	if(address >= 0 && address < MEMORY_SIZE) value = (int8_t) cpu->memory[address];
	else fprintf(stderr, "[!] memory_read_8() : invalid memory address @ %d\n", address);

	return(value);
}

void memory_write(CPU *cpu, int address, int32_t value)
{
	uint32_t raw_value = (uint32_t) value;

	if(address >= 0 && address + 3 < MEMORY_SIZE)
	{
		cpu->memory[address] = (uint8_t) (raw_value >> 24);
		cpu->memory[address + 1] = (uint8_t) (raw_value >> 16);
		cpu->memory[address + 2] = (uint8_t) (raw_value >> 8);
		cpu->memory[address + 3] = (uint8_t) raw_value;
	}
	else fprintf(stderr, "[!] memory_write() : invalid memory address @ %d\n", address);
}