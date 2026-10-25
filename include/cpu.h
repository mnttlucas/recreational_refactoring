#ifndef	CPU_H
#define	CPU_H

#include <stdint.h>

#define INVALID_PC UINT32_MAX
#define	MEMORY_SIZE (256 * 4)
#define	REGISTER_COUNT 32
#define WORD_SIZE 4
#define WORD_ALIGN_MASK (WORD_SIZE - 1u)	/* the low bits of an address aligned on a word are zero */

typedef struct
{
	uint32_t GPR[REGISTER_COUNT];
	uint32_t next_PC; /* Handle delay slot, field is INVALID_PC when no delay slot is pending */
	uint32_t PC;
	uint64_t FPR[REGISTER_COUNT];
	uint8_t memory[MEMORY_SIZE];
} Cpu;

void cpu_init(Cpu *cpu);

#endif	/* CPU_H */
