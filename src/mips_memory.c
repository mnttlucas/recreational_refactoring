#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "mips_memory.h"

void memory_init(Cpu *cpu)
{
	for(int i = 0; i < MEMORY_SIZE; i++) cpu->memory[i] = 0;
}

uint16_t memory_read_16(Cpu *cpu, uint32_t address)
{
	uint16_t value = 0;

	if(address < MEMORY_SIZE - 1)
	{
		value = (uint16_t) (cpu->memory[address] << 8);
		value |= (uint16_t) cpu->memory[address + 1];
	}
	else fprintf(stderr, "[!] memory_read_16() : invalid memory address @ %08" PRIx32 " (TODO : should signal an exception)\n", address);

	return(value);
}

uint32_t memory_read_32(Cpu *cpu, uint32_t address)
{
	uint32_t value = 0;

	if(address < MEMORY_SIZE - 3)
	{
		value = (uint32_t) cpu->memory[address] << 24;
		value |= (uint32_t) cpu->memory[address + 1] << 16;
		value |= (uint32_t) cpu->memory[address + 2] << 8;
		value |= (uint32_t) cpu->memory[address + 3];
	}
	else fprintf(stderr, "[!] memory_read_32() : invalid memory address @ %08" PRIx32 " (TODO : should signal an exception)\n", address);

	return(value);
}

/* Big-endian doubleword : the most significant word is at the lowest address */
uint64_t memory_read_64(Cpu *cpu, uint32_t address)
{
	uint64_t value = 0;

	if(address < MEMORY_SIZE - 7)
	{
		value = (uint64_t) memory_read_32(cpu, address) << 32;
		value |= (uint64_t) memory_read_32(cpu, address + 4);
	}
	else fprintf(stderr, "[!] memory_read_64() : invalid memory address @ %08" PRIx32 " (TODO : should signal an exception)\n", address);

	return(value);
}

uint8_t memory_read_8(Cpu *cpu, uint32_t address)
{
	uint8_t value = 0;

	if(address < MEMORY_SIZE) value = cpu->memory[address];
	else fprintf(stderr, "[!] memory_read_8() : invalid memory address @ %08" PRIx32 " (TODO : should signal an exception)\n", address);

	return(value);
}

void memory_write_16(Cpu *cpu, uint32_t address, uint16_t value)
{
	if(address < MEMORY_SIZE - 1)
	{
		cpu->memory[address] = (uint8_t) (value >> 8);
		cpu->memory[address + 1] = (uint8_t) value;
	}
	else fprintf(stderr, "[!] memory_write_16() : invalid memory address @ %08" PRIx32 " (TODO : should signal an exception)\n", address);
}

void memory_write_32(Cpu *cpu, uint32_t address, uint32_t value)
{
	if(address < MEMORY_SIZE - 3)
	{
		cpu->memory[address] = (uint8_t) (value >> 24);
		cpu->memory[address + 1] = (uint8_t) (value >> 16);
		cpu->memory[address + 2] = (uint8_t) (value >> 8);
		cpu->memory[address + 3] = (uint8_t) value;
	}
	else fprintf(stderr, "[!] memory_write_32() : invalid memory address @ %08" PRIx32 " (TODO : should signal an exception)\n", address);
}

void memory_write_64(Cpu *cpu, uint32_t address, uint64_t value)
{
	if(address < MEMORY_SIZE - 7)
	{
		memory_write_32(cpu, address, (uint32_t) (value >> 32));
		memory_write_32(cpu, address + 4, (uint32_t) value);
	}
	else fprintf(stderr, "[!] memory_write_64() : invalid memory address @ %08" PRIx32 " (TODO : should signal an exception)\n", address);
}

void memory_write_8(Cpu *cpu, uint32_t address, uint8_t value)
{
	if(address < MEMORY_SIZE) cpu->memory[address] = value;
	else fprintf(stderr, "[!] memory_write_8() : invalid memory address @ %08" PRIx32 " (TODO : should signal an exception)\n", address);
}
