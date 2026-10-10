#ifndef MODES_H
#define MODES_H

#include <stdio.h>

#include "config.h"
#include "cpu.h"
#include "instruction.h"

#define INTERACTIVE 0
#define BATCH       1

#define INITIAL_CAPACITY 64

typedef struct
{
	instruction *items;
	uint32_t count;
	size_t capacity;
} Program;

int load_program(const char *path, Program *prog);
FILE *open_file(const char *path, const char *mode);
int program_grow(Program *prog);
void run_program(CPU *cpu, Config *cfg, const Program *prog);
void write_hex(FILE *out, const Program *prog);
void write_regs(FILE *out, CPU *cpu);

void interactive_mode(CPU *cpu, Config *cfg);
void batch_mode(CPU *cpu, Config *cfg, char *path_in, char *path_out_hex, char *path_out_regs);

#endif /* MODES_H */