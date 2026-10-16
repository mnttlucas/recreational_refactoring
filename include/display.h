#ifndef	DISPLAY_H
#define	DISPLAY_H

#include "cpu.h"
#include "instruction.h"

#define DUMP_LINE_SIZE     4
#define DUMP_MEMORY_WORDS 40

void clear_output(void);
void cpu_dump(Cpu *cpu);
void log_instruction(Instruction *instr);
void log_usage(char *prog_name);
void wait_for_enter(void);

#endif	/* DISPLAY_H */
