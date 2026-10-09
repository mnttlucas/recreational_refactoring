#ifndef	CPU_H
#define	CPU_H

#include <stdint.h>

#define INVALID_PC UINT32_MAX
#define	MEMORY_SIZE (256 * 4)
#define	REGISTER_COUNT 32
#define WORD_SIZE 4

typedef struct 
{
    uint64_t FPR[REGISTER_COUNT];
    uint32_t GPR[REGISTER_COUNT];
    uint32_t PC;
    /* Handle delay slot, field is -1 when no delay slot is pending */
    uint32_t next_PC;
    uint8_t memory[MEMORY_SIZE];
} CPU;

#endif	/* CPU_H */