#ifndef	UTILS_H
#define	UTILS_H

#include <stdio.h>
#include <stdint.h>

#define ARR_SIZE(arr) \
	(sizeof(arr) / sizeof((arr)[0]))

int mystrcasecmp(const char *a, const char *b);
char *read_full_line(FILE *in);
int32_t sign_extend(int32_t value, uint8_t bits);
uint32_t low_mask(unsigned bits);

typedef enum
{
	EXC_ADDRESS_ERROR,
	EXC_FP_INVALID_OPERATION,
	EXC_FP_UNIMPLEMENTED_OPERATION,
	EXC_INTEGER_OVERFLOW,
	EXC_TRAP
} Exception;

void signal_exception(Exception exception);

#endif	/* UTILS_H */
