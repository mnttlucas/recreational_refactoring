#ifndef	EXECUTE_H
#define	EXECUTE_H

#include "config.h"
#include "cpu.h"
#include "instruction.h"

void execute_instruction(Cpu *cpu, Config *cfg, Instruction instr);

#endif	/* EXECUTE_H */
