#ifndef	REGISTERS_H
#define	REGISTERS_H

#include "cpu.h"

#define	REG_HI 32
#define	REG_LO 33
#define	REG_PC 34

void	registers_init(CPU *cpu);
long	register_read(CPU *cpu, int id);
void	register_write(CPU *cpu, int id, long value);

#endif	/* REGISTERS_H */