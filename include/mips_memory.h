#ifndef	MIPS_MEMORY_H
#define	MIPS_MEMORY_H

#include <stdint.h>

#include "cpu.h"

void memory_init(CPU *cpu);
uint16_t memory_read_16(CPU *cpu, uint32_t address);
uint32_t memory_read_32(CPU *cpu, uint32_t address);
uint8_t memory_read_8(CPU *cpu, uint32_t address);
void memory_write_16(CPU *cpu, uint32_t address, uint16_t value);
void memory_write_32(CPU *cpu, uint32_t address, uint32_t value);
void memory_write_8(CPU *cpu, uint32_t address, uint8_t value);

#endif	/* MIPS_MEMORY_H */