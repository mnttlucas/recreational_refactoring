#ifndef	REGISTERS_H
#define	REGISTERS_H

#include <stdint.h>

#include "cpu.h"

#define	REG_HI 32
#define	REG_LO 33
#define	REG_PC 34

void	registers_init(CPU *cpu);
int32_t	register_read(CPU *cpu, int id);
void	register_write(CPU *cpu, int id, int32_t value);

#endif	/* REGISTERS_H */