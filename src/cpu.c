#include "cpu.h"
#include "mips_memory.h"
#include "mips_registers.h"

void cpu_init(Cpu *cpu)
{
	memory_init(cpu);
	registers_init(cpu);

	cpu->next_PC = INVALID_PC;
	cpu->PC = 0;
}
