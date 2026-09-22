#ifndef	CPU_H
#define	CPU_H

#define	MEMORY_SIZE 256 * 4
#define	REGISTER_COUNT 35
#define WORD_SIZE 4

#include <stdint.h>

typedef struct 
{
    uint8_t memory[MEMORY_SIZE];
    int32_t registers[REGISTER_COUNT];
    /* Handle delay slot, field is -1 when no delay slot is pending */
    int32_t next_PC;
} CPU;

#endif	/* CPU_H */