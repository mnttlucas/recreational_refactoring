#ifndef	EXECUTE_H
#define	EXECUTE_H

#include "instruction.h"

void	execute_instruction(CPU *cpu, config *cfg, instruction instr);
void    increment_pc(CPU *cpu);

#endif	/* EXECUTE_H */