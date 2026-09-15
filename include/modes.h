#ifndef MODES_H
#define MODES_H

#include "config.h"
#include "cpu.h"

void interactive_mode(CPU *cpu, Config *cfg);
void batch_mode(CPU *cpu, Config *cfg, char *path_in, char *path_out_hex, char *path_out_regs);

#endif /* MODES_H */