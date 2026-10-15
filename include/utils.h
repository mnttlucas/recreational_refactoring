#ifndef	UTILS_H
#define	UTILS_H

#include <stdio.h>
#include <stdint.h>

#define ARR_SIZE(arr) \
	(sizeof(arr) / sizeof((arr)[0]))

int mystrcasecmp(const char *a, const char *b);
char *read_full_line(FILE *in);
int32_t sign_extend(int32_t value, uint8_t bits);

#endif	/* UTILS_H */
