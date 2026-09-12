#ifndef	CPU_H
#define	CPU_H

#define	MEMORY_SIZE 20
#define	REGISTER_COUNT 35

#include <stdint.h>

typedef struct 
{
    int32_t memory[MEMORY_SIZE];
    int32_t registers[REGISTER_COUNT];
    /* Handle delay slot, field is -1 when no delay slot is pending */
    int32_t next_PC;
} CPU;

#endif	/* CPU_H */