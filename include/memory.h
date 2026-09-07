#ifndef MEMORY_H
#define MEMORY_H

#include "cpu.h"

void	memory_init(CPU *cpu);
int		memory_read(CPU *cpu, int id);
void	memory_write(CPU *cpu, int id, int value);

#endif  /* MEMORY_H */