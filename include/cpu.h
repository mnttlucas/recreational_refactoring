#ifndef	CPU_H
#define	CPU_H

#include <stdint.h>

#include "exception.h"

#define INVALID_PC UINT32_MAX
#define	MEMORY_SIZE (256 * 4)
#define	REGISTER_COUNT 32
#define WORD_SIZE 4
#define WORD_ALIGN_MASK (WORD_SIZE - 1u)	/* the low bits of an address aligned on a word are zero */

/* What the next instruction is, relative to the branch that was just executed : a control transfer
   instruction (a branch or a jump) is not allowed there, it signals a Reserved Instruction exception */
typedef enum
{
	SLOT_NONE,
	SLOT_DELAY,		/* delay slot of a branch or jump (taken or not) */
	SLOT_FORBIDDEN	/* forbidden slot of a compact branch that was not taken */
} SlotKind;

typedef struct
{
	SlotKind slot;
	ExceptionState exception;
	uint32_t GPR[REGISTER_COUNT];
	uint32_t next_PC; /* Handle delay slot, field is INVALID_PC when no delay slot is pending */
	uint32_t PC;
	uint64_t FPR[REGISTER_COUNT];
	uint8_t memory[MEMORY_SIZE];
} Cpu;

void cpu_init(Cpu *cpu);
void raise_exception(Cpu *cpu, Exception exception, uint32_t bad_vaddr);

#endif	/* CPU_H */
