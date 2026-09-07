#ifndef EXECUTE_H
#define EXECUTE_H

#include "instruction.h"

void execute_instruction(CPU *cpu, config *cfg, instruction instr);

#endif /* EXECUTE_H */