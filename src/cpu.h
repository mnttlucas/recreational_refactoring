#ifndef	CPU_H
#define	CPU_H

#define	MEMORY_SIZE 20
#define	REGISTER_COUNT 35

typedef struct 
{
    int	memory[MEMORY_SIZE];
    long registers[REGISTER_COUNT];
} CPU;

#endif	/* CPU_H */