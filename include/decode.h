#ifndef	DECODE_H
#define	DECODE_H

#include <stdint.h>
#include <stdio.h>

#include "instruction.h"

instruction	decode_instruction(int mode, FILE *input);

#endif	/* DECODE_H */