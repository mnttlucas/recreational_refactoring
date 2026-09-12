#include <stdint.h>
#include <stdio.h>

#include "config.h"
#include "cpu.h"
#include "execute.h"
#include "memory.h"
#include "registers.h"
#include "utils.h"

void execute_instruction(CPU *cpu, config *cfg, instruction instr)
{
	int32_t address, dividend, divisor, pending_PC, res_32, res_HI, res_LO;
	int64_t res_64;
	uint32_t raw_32;

	pending_PC = cpu->next_PC;
	cpu->next_PC = -1;
	
	switch(instr.opcode)
	{
		case ADD :
			res_64 = (int64_t) register_read(cpu, instr.rs) + (int64_t) register_read(cpu, instr.rt);
			if(res_64 > INT32_MAX || res_64 < INT32_MIN)
				fprintf(stderr, "[!] Exception : Integer Overflow\n");
			else
				register_write(cpu, instr.rd, (int32_t) res_64);
			break;
		case ADDI :
			res_64 = (int64_t) register_read(cpu, instr.rs) + (int64_t) instr.immediate;
			if(res_64 > INT32_MAX || res_64 < INT32_MIN)
				fprintf(stderr, "[!] Exception : Integer Overflow\n");
			else
				register_write(cpu, instr.rt, (int32_t) res_64);
			break;
		case AND :
			res_32 = register_read(cpu, instr.rs) & register_read(cpu, instr.rt);
			register_write(cpu, instr.rd, res_32);
			break;
		case BEQ :
			if(register_read(cpu, instr.rs) == register_read(cpu, instr.rt))
				cpu->next_PC = register_read(cpu, REG_PC) + instr.offset + 1;
			break;
		case BGTZ :
			if(register_read(cpu, instr.rs) > 0)
				cpu->next_PC = register_read(cpu, REG_PC) + instr.offset + 1;
			break;
		case BLEZ :
			if(register_read(cpu, instr.rs) <= 0)
				cpu->next_PC = register_read(cpu, REG_PC) + instr.offset + 1;
			break;
		case BNE :
			if(register_read(cpu, instr.rs) != register_read(cpu, instr.rt))
				cpu->next_PC = register_read(cpu, REG_PC) + instr.offset + 1;
			break;
		case DIV :
			dividend = register_read(cpu, instr.rs);
			divisor = register_read(cpu, instr.rt);
			/* Arbitrary choice, MIPS32 documentation :
			'If the divisor in GPR rt is zero, the arithmetic result value is UNPREDICTABLE' */
			if(divisor == 0)
			{
				res_HI = dividend;
				res_LO = divisor;
			}
			/* Used to cover a C edge-case of integer overflow - not MIPS related */
			else if(divisor == -1 && dividend == (int32_t) 0x80000000)
			{
				res_HI = 0;
				res_LO = dividend;
			}
			else
			{
				res_HI = dividend % divisor;
				res_LO = dividend / divisor;
			}
			register_write(cpu, REG_LO, res_LO);
			register_write(cpu, REG_HI, res_HI);
			break;
		case J :
			cpu->next_PC = instr.target;
			break;
		case JAL :
			register_write(cpu, REG_RA, register_read(cpu, REG_PC) + 2);
			cpu->next_PC = instr.target;
			break;
		case JR :
			cpu->next_PC = register_read(cpu, instr.rs);
			break;
		case LUI :
			res_32 = instr.immediate << 16;
			register_write(cpu, instr.rt, res_32);
			break;
		case LW :
			address = register_read(cpu, instr.base) + instr.offset;
			res_32 = memory_read(cpu, (int) address);
			register_write(cpu, instr.rt, res_32);
			break;
		case MFHI :
			res_32 = register_read(cpu, REG_HI);
			register_write(cpu, instr.rd, res_32);
			break;
		case MFLO :
			res_32 = register_read(cpu, REG_LO);
			register_write(cpu, instr.rd, res_32);
			break;
		case MULT :
			res_64 = (int64_t) register_read(cpu, instr.rs) * (int64_t) register_read(cpu, instr.rt);
			res_LO = (int32_t) (uint32_t) (res_64 & 0xFFFFFFFF);
			res_HI = (int32_t) (uint32_t) ((res_64 >> 32) & 0xFFFFFFFF);
			register_write(cpu, REG_HI, res_HI);
			register_write(cpu, REG_LO, res_LO);
			break;
		case NOP :
			break;
		case OR :
			res_32 = register_read(cpu, instr.rs) | register_read(cpu, instr.rt);
			register_write(cpu, instr.rd, res_32);
			break;
		case ROTR :
			raw_32 = (uint32_t) register_read(cpu, instr.rt);
			res_32 = (int32_t) (raw_32 >> instr.sa | raw_32 << ((32 - instr.sa) & 31));
			register_write(cpu, instr.rd, res_32);
			break;
		case SLL :
			res_32 = register_read(cpu, instr.rt) << instr.sa;
			register_write(cpu, instr.rd, res_32);
			break;
		case SLT :
			if(register_read(cpu, instr.rs) < register_read(cpu, instr.rt))
				register_write(cpu, instr.rd, 1);
			else
				register_write(cpu, instr.rd, 0);
			break;
		case SRL :
			raw_32 = (uint32_t) register_read(cpu, instr.rt);
			res_32 = (int32_t) (raw_32 >> instr.sa);
			register_write(cpu, instr.rd, res_32);
			break;
		case SUB :
			res_64 = (int64_t) register_read(cpu, instr.rs) - (int64_t) register_read(cpu, instr.rt);
			if(res_64 > INT32_MAX || res_64 < INT32_MIN)
				fprintf(stderr, "[!] Exception : Integer Overflow\n");
			else
				register_write(cpu, instr.rd, (int32_t) res_64);
			break;
		case SW :
			res_32 = register_read(cpu, instr.rt);
			address = register_read(cpu, instr.base) + instr.offset;
			memory_write(cpu, (int) address, res_32);
			break;
		case XOR :
			res_32 = register_read(cpu, instr.rs) ^ register_read(cpu, instr.rt);
			register_write(cpu, instr.rd, (int32_t) res_32);
			break;
		default :
			fprintf(stderr, "[!] This line was either a comment or an unknown command\n");
			break;
	}

	increment_pc(cpu);

	if(pending_PC != -1)
		register_write(cpu, REG_PC, pending_PC);

	if(cfg->verbose)
		printf("%s", instr.toString);
}