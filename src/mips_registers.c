#include <stdint.h>
#include <stdio.h>

#include "cpu.h"
#include "mips_registers.h"

void registers_init(CPU *cpu)
{
	for(int i = 0; i < REGISTER_COUNT; i++) cpu->GPR[i] = 0;
	cpu->PC = 0;
	cpu->next_PC = INVALID_PC;
}

uint32_t register_read(CPU *cpu, int id)
{
	uint32_t value = 0;

	if(id >= 0 && id < REGISTER_COUNT) value = cpu->GPR[id];
	else fprintf(stderr, "[!] register_read() : invalid register %d\n", id);
	
	return(value);
}

void register_write(CPU *cpu, int id, uint32_t value)
{
	if(id > 0 && id < REGISTER_COUNT) cpu->GPR[id] = value;
	else if(id == 0);
	else fprintf(stderr, "[!] register_write() : invalid register %d\n", id);
}