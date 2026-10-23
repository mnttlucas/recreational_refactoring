#ifndef	DECODE_H
#define	DECODE_H

#include <stdio.h>

#include "instruction.h"

#define BATCH       0
#define INTERACTIVE 1

Instruction	decode_instruction(int mode, FILE *input);

#endif	/* DECODE_H */
