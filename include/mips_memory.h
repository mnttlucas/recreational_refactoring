#ifndef	MIPS_MEMORY_H
#define	MIPS_MEMORY_H

#include <stdint.h>

#include "cpu.h"

void	memory_init(CPU *cpu);
int32_t	memory_read(CPU *cpu, int id);
void	memory_write(CPU *cpu, int id, int32_t value);

#endif	/* MIPS_MEMORY_H */