#ifndef	DECODE_H
#define	DECODE_H

#include <stdio.h>

#include "instruction.h"

#define BATCH       0
#define INTERACTIVE 1

Instruction	decode_instruction(int mode, FILE *input, int hex);
void		decode_assemble(const InstructionDesc *id, char *operands, Instruction *instr);
void		decode_set_quiet(int mute);

#endif	/* DECODE_H */
