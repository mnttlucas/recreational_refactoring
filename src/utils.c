#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

static int to_lower_ascii(int c)
{
	return((c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c);
}

int mystrcasecmp(const char *a, const char *b)
{
	while(*a && to_lower_ascii((unsigned char) *a) == to_lower_ascii((unsigned char) *b))
	{
		a++;
		b++;
	}

	return(to_lower_ascii((unsigned char) *a) - to_lower_ascii((unsigned char) *b));
}

char *read_full_line(FILE *in)
{
	size_t capacity = 256, length = 0;
	char *line = malloc(capacity);

	if(!line) return(NULL);

	while(fgets(line + length, (int) (capacity - length), in))
	{
		length += strlen(line + length);

		if(length > 0 && line[length - 1] == '\n') return(line);
		if(feof(in)) return(line);

		capacity *= 2;
		char *tmp = realloc(line, capacity);
		if(!tmp)
		{
			free(line);
			return(NULL);
		}
		line = tmp;
	}

	if(length == 0)
	{
		free(line);
		return(NULL);
	}

	return(line);
}

int32_t sign_extend(int32_t value, uint8_t bits)
{
	uint32_t raw = (uint32_t) value << (32 - bits);
	return((int32_t) raw >> (32 - bits));
}

/* The `bits` low bits set (0 to 32) */
uint32_t low_mask(unsigned bits)
{
	return(bits >= 32 ? UINT32_MAX : (UINT32_C(1) << bits) - 1);
}

/* The names of the exceptions of the manual, in the order of the Exception enum */
static const char *const exception_names[] =
{
	"Address Error",
	"Floating Point Invalid Operation",
	"Floating Point Unimplemented Operation",
	"Integer Overflow",
	"Reserved Instruction",
	"Trap"
};

/* TODO : there is no exception mechanism yet, an exception only prints a message and the program goes on.
   Every exception of the emulator goes through here, to be replaced once exceptions are really handled. */
void signal_exception(Exception exception)
{
	fprintf(stderr, "[!] Exception : %s (TODO : not handled yet, execution goes on)\n", exception_names[exception]);
}
