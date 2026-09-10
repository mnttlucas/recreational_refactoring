#include <limits.h>
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
	int32_t dividend, divisor, res_HI, res_LO;
	int64_t res;
	long address;
	
	switch(instr.opcode)
	{
		case ADD :
			res = register_read(cpu, instr.rs) + register_read(cpu, instr.rt);
			if(res > INT32_MAX || res < INT32_MIN)
				fprintf(stderr, "[!] Exception : Integer Overflow\n");
			else
				register_write(cpu, instr.rd, (int32_t) res);
			increment_pc(cpu);
			break;
		case ADDI :
			res = register_read(cpu, instr.rs) + instr.immediate;
			if(res > INT32_MAX || res < INT32_MIN)
				fprintf(stderr, "[!] Exception : Integer Overflow\n");
			else
				register_write(cpu, instr.rt, (int32_t) res);
			increment_pc(cpu);
			break;
		case AND :
			res = register_read(cpu, instr.rs) & register_read(cpu, instr.rt);
			register_write(cpu, instr.rd, (int32_t) res);
			increment_pc(cpu);
			break;
		case BEQ :
			if(register_read(cpu, instr.rs) == register_read(cpu, instr.rt))
				register_write(cpu, REG_PC, register_read(cpu, REG_PC) + instr.offset);
			else
				increment_pc(cpu);
			break;
		case BGTZ :
			if(register_read(cpu, instr.rs) > 0)
				register_write(cpu, REG_PC, register_read(cpu, REG_PC) + instr.offset);
			else
				increment_pc(cpu);
			break;
		case BLEZ :
			if(register_read(cpu, instr.rs) <= 0)
				register_write(cpu, REG_PC, register_read(cpu, REG_PC) + instr.offset);
			else
				increment_pc(cpu);
			break;
		case BNE :
			if(register_read(cpu, instr.rs) != register_read(cpu, instr.rt))
				register_write(cpu, REG_PC, register_read(cpu, REG_PC) + instr.offset);
			else
				increment_pc(cpu);
			break;
		case DIV :
			dividend = register_read(cpu, instr.rs);
			divisor = register_read(cpu, instr.rt);
			if(divisor == 0)
			{
				res_HI = dividend;
				res_LO = divisor;
			}
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
			increment_pc(cpu);
			break;
		case J :
			register_write(cpu, REG_PC, instr.target);
			break;
		case JAL :
			register_write(cpu, 31, register_read(cpu, REG_PC) + 1);
			register_write(cpu, REG_PC, instr.target);
			break;
		case JR :
			register_write(cpu, REG_PC, register_read(cpu, instr.rs));
			break;
		case LUI :
			res = instr.immediate << 16;
			register_write(cpu, instr.rt, (int32_t) res);
			increment_pc(cpu);
			break;
		case LW :
			address = register_read(cpu, instr.base) + instr.offset;
			res = memory_read(cpu, (int) address);
			register_write(cpu, instr.rt, (int32_t) res);
			increment_pc(cpu);
			break;
		case MFHI :
			res = register_read(cpu, REG_HI);
			register_write(cpu, instr.rd, (int32_t) res);
			increment_pc(cpu);
			break;
		case MFLO :
			res = register_read(cpu, REG_LO);
			register_write(cpu, instr.rd, (int32_t) res);
			increment_pc(cpu);
			break;
		case MULT :
			res_LO = (register_read(cpu, instr.rs) * register_read(cpu, instr.rt)) & 65535;
			res_HI = register_read(cpu, instr.rs) * register_read(cpu, instr.rt) - res_LO;
			register_write(cpu, REG_HI, (int32_t) res_HI);
			register_write(cpu, REG_LO, (int32_t) res_LO);
			increment_pc(cpu);
			break;
		case NOP :
			increment_pc(cpu);
			break;
		case OR :
			res = register_read(cpu, instr.rs) | register_read(cpu, instr.rt);
			register_write(cpu, instr.rd, (int32_t) res);
			increment_pc(cpu);
			break;
		case ROTR :
			res = register_read(cpu, instr.rt);
			res = res >> instr.sa | res << ((32 - instr.sa) & 31);
			register_write(cpu, instr.rd, (int32_t) res);
			increment_pc(cpu);
			break;
		case SLL :
			res = register_read(cpu, instr.rt);
			for(int i = 0; i < instr.sa; i++)
				res = (res * 2) & 8589934591;
			register_write(cpu, instr.rd, (int32_t) res);
			increment_pc(cpu);
			break;
		case SLT :
			if(register_read(cpu, instr.rs) < register_read(cpu, instr.rt))
				register_write(cpu, instr.rd, 1);
			else
				register_write(cpu, instr.rd, 0);
			increment_pc(cpu);
			break;
		case SRL :
			res = register_read(cpu, instr.rt);
			for(int i = 0; i < instr.sa; i++)
				res /= 2;
			register_write(cpu, instr.rd, (int32_t) res);
			increment_pc(cpu);
			break;
		case SUB :
			res = register_read(cpu, instr.rs) - register_read(cpu, instr.rt);
			register_write(cpu, instr.rd, (int32_t) res);
			increment_pc(cpu);
			break;
		case SW :
			res = register_read(cpu, instr.rt);
			address = register_read(cpu, instr.base) + instr.offset;
			memory_write(cpu, (int) address, (int32_t)  res);
			increment_pc(cpu);
			break;
		case XOR :
			res = register_read(cpu, instr.rs) ^ register_read(cpu, instr.rt);
			register_write(cpu, instr.rd, (int32_t) res);
			increment_pc(cpu);
			break;
		default :
			break;
	}

	if(cfg->verbose)
		printf("%s", instr.toString);
}