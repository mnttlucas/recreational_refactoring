#ifndef	UTILS_H
#define	UTILS_H

#include <stdio.h>

#include "config.h"
#include "cpu.h"
#include "instruction.h"

#define DUMP_LINE_SIZE     4
#define DUMP_MEMORY_WORDS 40

#define ARR_SIZE(arr) \
	(sizeof(arr) / sizeof((arr)[0]))

void clear_output();
void cpu_dump(CPU *cpu, Config *cfg);
void log_instruction(instruction *instr);
void log_usage(char *prog_name);
char *read_full_line(FILE *in);
uint8_t register_string_to_int(char *str);
int32_t sign_extend(int32_t value, uint8_t bits);
void wait_for_enter();

#endif	/* UTILS_H */