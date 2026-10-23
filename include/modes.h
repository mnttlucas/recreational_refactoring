#ifndef MODES_H
#define MODES_H

#include "config.h"
#include "cpu.h"
#include "instruction.h"

int batch_mode(Cpu *cpu, Config *cfg, char *path_in, char *path_out_hex, char *path_out_regs);
void interactive_mode(Cpu *cpu, Config *cfg);

#endif /* MODES_H */
