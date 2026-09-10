#ifndef	CPU_H
#define	CPU_H

#define	MEMORY_SIZE 20
#define	REGISTER_COUNT 35

#include <stdint.h>

typedef struct 
{
    int32_t memory[MEMORY_SIZE];
    int32_t registers[REGISTER_COUNT];
} CPU;

#endif	/* CPU_H */