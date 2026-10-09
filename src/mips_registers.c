#include <stdint.h>
#include <stdio.h>

#include "cpu.h"
#include "mips_registers.h"

void registers_init(CPU *cpu)
{
	for(int i = 0; i < REGISTER_COUNT; i++)
	{
		cpu->FPR[i] = 0;
		cpu->GPR[i] = 0;
	}
	cpu->PC = 0;
	cpu->next_PC = INVALID_PC;
}

uint32_t fpr_read_32(CPU *cpu, int id)
{
	uint32_t value = 0;

	if(id >= 0 && id < REGISTER_COUNT) value = (uint32_t) cpu->FPR[id];
	else fprintf(stderr, "[!] fpr_read_32() : invalid register %d\n", id);

	return(value);
}

uint64_t fpr_read_64(CPU *cpu, int id)
{
	uint64_t value = 0;

	if(id >= 0 && id < REGISTER_COUNT) value = cpu->FPR[id];
	else fprintf(stderr, "[!] fpr_read_64() : invalid register %d\n", id);

	return(value);
}

void fpr_write_32(CPU *cpu, int id, uint32_t value)
{
	if(id >= 0 && id < REGISTER_COUNT) cpu->FPR[id] = (uint64_t) value;
	else fprintf(stderr, "[!] fpr_write_32() : invalid register %d\n", id);
}

void fpr_write_64(CPU *cpu, int id, uint64_t value)
{
	if(id >= 0 && id < REGISTER_COUNT) cpu->FPR[id + 1] = value;
	else fprintf(stderr, "[!] fpr_write_64() : invalid register %d\n", id);
}

uint32_t gpr_read_32(CPU *cpu, int id)
{
	uint32_t value = 0;

	if(id >= 0 && id < REGISTER_COUNT) value = cpu->GPR[id];
	else fprintf(stderr, "[!] gpr_read_32() : invalid register %d\n", id);
	
	return(value);
}

void gpr_write_32(CPU *cpu, int id, uint32_t value)
{
	if(id > 0 && id < REGISTER_COUNT) cpu->GPR[id] = value;
	else if(id == 0);
	else fprintf(stderr, "[!] gpr_write_32() : invalid register %d\n", id);
}