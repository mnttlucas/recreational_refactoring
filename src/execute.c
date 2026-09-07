#include <stdio.h>

#include "config.h"
#include "cpu.h"
#include "execute.h"
#include "memory.h"
#include "registers.h"

void execute_instruction(CPU *cpu, config *cfg, instruction instr)
{
	long address, res, res_HI, res_LO, shift_amount;

	switch(instr.opcode)
	{
		case ADD :
			res = register_read(cpu, instr.rs) + register_read(cpu, instr.rt);
			register_write(cpu, instr.rd, res);
			register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
			break;
		case ADDI :
			res = register_read(cpu, instr.rs) + instr.immediate;
			register_write(cpu, instr.rt, res);
			register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
			break;
		case AND :
			res = register_read(cpu, instr.rs) & register_read(cpu, instr.rt);
			register_write(cpu, instr.rd, res);
			register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
			break;
		case BEQ :
			if(register_read(cpu, instr.rs) == register_read(cpu, instr.rt))
				register_write(cpu, REG_PC, register_read(cpu, REG_PC) + instr.offset);
			else
				register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
			break;
		case BGTZ :
			if(register_read(cpu, instr.rs) > 0)
				register_write(cpu, REG_PC, register_read(cpu, REG_PC) + instr.offset);
			else
				register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
			break;
		case BLEZ :
			if(register_read(cpu, instr.rs) <= 0)
				register_write(cpu, REG_PC, register_read(cpu, REG_PC) + instr.offset);
			else
				register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
			break;
		case BNE :
			if(register_read(cpu, instr.rs) != register_read(cpu, instr.rt))
				register_write(cpu, REG_PC, register_read(cpu, REG_PC) + instr.offset);
			else
				register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
			break;
		case DIV :
			if(register_read(cpu, instr.rt) != 0)
			{
				res_HI = register_read(cpu, instr.rs) / register_read(cpu, instr.rt);
				res_LO = register_read(cpu, instr.rs) % register_read(cpu, instr.rt);
				register_write(cpu, REG_HI, res_HI);
				register_write(cpu, REG_LO, res_LO);
			}
			register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
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
			res = register_read(cpu, instr.immediate) * 65536;
			register_write(cpu, instr.rt, res);
			register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
			break;
		case LW :
			address = register_read(cpu, instr.base) + instr.offset;
			res = memory_read(cpu, (int) address);
			register_write(cpu, instr.rt, res);
			register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
			break;
		case MFHI :
			res = register_read(cpu, REG_HI);
			register_write(cpu, instr.rd, res);
			register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
			break;
		case MFLO :
			res = register_read(cpu, REG_LO);
			register_write(cpu, instr.rd, res);
			register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
			break;
		case MULT :
			res_LO = (register_read(cpu, instr.rs) * register_read(cpu, instr.rt)) & 65535;
			res_HI = register_read(cpu, instr.rs) * register_read(cpu, instr.rt) - res_LO;
			register_write(cpu, REG_HI, res_HI);
			register_write(cpu, REG_LO, res_LO);
			register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
			break;
		case NOP :
			register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
			break;
		case OR :
			res = register_read(cpu, instr.rs) | register_read(cpu, instr.rt);
			register_write(cpu, instr.rd, res);
			register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
			break;
		case ROTR :
			shift_amount = register_read(cpu, instr.sa);
			res = register_read(cpu, instr.rt);
			res = res >> shift_amount | res << (32 - shift_amount);
			register_write(cpu, instr.rd, res);
			register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
			break;
		case SLL :
			shift_amount = register_read(cpu, instr.sa);
			res = register_read(cpu, instr.rt);
			for(int i = 0; i < shift_amount; i++)
				res = (res * 2) & 8589934591;
			register_write(cpu, instr.rd, res);
			register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
			break;
		case SLT :
			if(register_read(cpu, instr.rs) < register_read(cpu, instr.rt))
				register_write(cpu, instr.rd, 1);
			else
				register_write(cpu, instr.rd, 0);
			register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
			break;
		case SRL :
			shift_amount = register_read(cpu, instr.sa);
			res = register_read(cpu, instr.rt);
			for(int i = 0; i < shift_amount; i++)
				res /= 2;
			register_write(cpu, instr.rd, res);
			register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
			break;
		case SUB :
			res = register_read(cpu, instr.rs) - register_read(cpu, instr.rt);
			register_write(cpu, instr.rd, res);
			register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
			break;
		case SW :
			res = register_read(cpu, instr.rt);
			address = register_read(cpu, instr.base) + instr.offset;
			memory_write(cpu, (int) address, (int) res);
			register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
			break;
		case XOR :
			res = register_read(cpu, instr.rs) ^ register_read(cpu, instr.rt);
			register_write(cpu, instr.rd, res);
			register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
			break;
		default :
			break;
	}

	if(cfg->verbose)
		printf("%s\n", instr.toString);
}