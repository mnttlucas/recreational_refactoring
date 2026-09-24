#ifndef	MIPS_MEMORY_H
#define	MIPS_MEMORY_H

#include <stdint.h>

#include "cpu.h"

void	memory_init(CPU *cpu);
int32_t	memory_read_32(CPU *cpu, int address);
int8_t	memory_read_8(CPU *cpu, int address);
void	memory_write(CPU *cpu, int address, int32_t value);

#endif	/* MIPS_MEMORY_H */