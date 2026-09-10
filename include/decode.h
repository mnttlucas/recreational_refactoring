#ifndef	DECODE_H
#define	DECODE_H

#include "instruction.h"

#define FIELD(start, end, value) \
	(instruction_field) {start, end, value}

typedef struct
{
	int  start_bit;
	int  end_bit;
	long value;
} instruction_field;

void build_instruction_bin(instruction_field *arr, size_t field_count, int *bin_arr, char *hex_arr);
instruction	decode_instruction(int mode, FILE *fichier);

#endif	/* DECODE_H */