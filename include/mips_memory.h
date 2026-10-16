#ifndef	MIPS_MEMORY_H
#define	MIPS_MEMORY_H

#include <stdint.h>

#include "cpu.h"

void memory_init(Cpu *cpu);
uint16_t memory_read_16(Cpu *cpu, uint32_t address);
uint32_t memory_read_32(Cpu *cpu, uint32_t address);
uint8_t memory_read_8(Cpu *cpu, uint32_t address);
void memory_write_16(Cpu *cpu, uint32_t address, uint16_t value);
void memory_write_32(Cpu *cpu, uint32_t address, uint32_t value);
void memory_write_8(Cpu *cpu, uint32_t address, uint8_t value);

#endif	/* MIPS_MEMORY_H */
