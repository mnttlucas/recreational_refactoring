#include <stdio.h>

#include "cpu.h"
#include "execute.h"
#include "memory.h"
#include "registers.h"

void instructionADD(CPU *cpu, instruction instr, int printable){
	long res;
	res = register_read(cpu, instr.rs) + register_read(cpu, instr.rt);
	register_write(cpu, instr.rd, res);
	register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
	if(printable){
		printf("ADD $%d,$%d,$%d -> 0x%s\n", instr.rd, instr.rs, instr.rt, instr.instrHex);
	}
}

void instructionADDI(CPU *cpu, instruction instr, int printable){
	long res;
	res = register_read(cpu, instr.rs) + instr.immediate;
	register_write(cpu, instr.rt, res);
	register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
	if(printable){
		printf("ADDI $%d,$%d,%d -> 0x%s\n", instr.rt, instr.rs, instr.immediate, instr.instrHex);
	}
}

void instructionAND(CPU *cpu, instruction instr, int printable){
	long res;
	res = register_read(cpu, instr.rs) & register_read(cpu, instr.rt);
	register_write(cpu, instr.rd, res);
	register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
	if(printable){
		printf("AND $%d,$%d,$%d -> 0x%s\n", instr.rd, instr.rs, instr.rt, instr.instrHex);
	}
}

void instructionBEQ(CPU *cpu, instruction instr, int printable){
	if(register_read(cpu, instr.rs) == register_read(cpu, instr.rt)){
		register_write(cpu, REG_PC, register_read(cpu, REG_PC) + instr.offset);
	}
	else{
		register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
	}
	if(printable){
		printf("BEQ $%d,$%d,%d -> 0x%s\n", instr.rs, instr.rt, instr.offset, instr.instrHex);
	}
}

void instructionBGTZ(CPU *cpu, instruction instr, int printable){
	if(register_read(cpu, instr.rs) > 0){
		register_write(cpu, REG_PC, register_read(cpu, REG_PC) + instr.offset);
	}
	else{
		register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
	}
	if(printable){
		printf("BGTZ $%d,%d -> 0x%s\n", instr.rs, instr.offset, instr.instrHex);
	}
}

void instructionBLEZ(CPU *cpu, instruction instr, int printable){
	if(register_read(cpu, instr.rs) <= 0){
		register_write(cpu, REG_PC, register_read(cpu, REG_PC) + instr.offset);
	}
	else{
		register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
	}
	if(printable){
		printf("BLEZ $%d,%d -> 0x%s\n", instr.rs, instr.offset, instr.instrHex);
	}
}

void instructionBNE(CPU *cpu, instruction instr, int printable){
	if(register_read(cpu, instr.rs) != register_read(cpu, instr.rt)){
		register_write(cpu, REG_PC, register_read(cpu, REG_PC) + instr.offset);
	}
	else{
		register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
	}
	if(printable){
		printf("BNE $%d,$%d,%d -> 0x%s\n", instr.rs, instr.rt, instr.offset, instr.instrHex);
	}
}

void instructionDIV(CPU *cpu, instruction instr, int printable){
	long resHI, resLO;
	if(register_read(cpu, instr.rt) != 0){
		resHI = register_read(cpu, instr.rs) / register_read(cpu, instr.rt);
		resLO = register_read(cpu, instr.rs) % register_read(cpu, instr.rt);
		register_write(cpu, REG_HI, resHI);
		register_write(cpu, REG_LO, resLO);
	}
	register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
	if(printable){
		printf("DIV $%d,$%d -> 0x%s\n", instr.rs, instr.rt, instr.instrHex);
	}
}

void instructionJ(CPU *cpu, instruction instr, int printable){
	register_write(cpu, REG_PC, instr.target);
	if(printable){
		printf("J %d -> 0x%s\n", instr.target, instr.instrHex);
	}
}

void instructionJAL(CPU *cpu, instruction instr, int printable){
	register_write(cpu, 31, register_read(cpu, REG_PC) + 1);
	register_write(cpu, REG_PC, instr.target);
	if(printable){
		printf("JAL %d -> 0x%s\n", instr.target, instr.instrHex);
	}
}

void instructionJR(CPU *cpu, instruction instr, int printable){
	register_write(cpu, REG_PC, register_read(cpu, instr.rs));
	if(printable){
		printf("JR $%d -> 0x%s\n", instr.rs, instr.instrHex);
	}
}

void instructionLUI(CPU *cpu, instruction instr, int printable){
	long res;
	res = register_read(cpu, instr.immediate) * 65536;
	register_write(cpu, instr.rt, res);
	register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
	if(printable){
		printf("LUI $%d,%d -> 0x%s\n", instr.rt, instr.immediate, instr.instrHex);
	}
}

void instructionLW(CPU *cpu, instruction instr, int printable){
	long res, mem;
	mem = register_read(cpu, instr.base) + instr.offset;
	res = memory_read(cpu, (int) mem);
	register_write(cpu, instr.rt, res);
	register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
	if(printable){
		printf("LW $%d,%d,($%d) -> 0x%s\n", instr.rt, instr.offset, instr.base, instr.instrHex);
	}
}

void instructionMFHI(CPU *cpu, instruction instr, int printable){
	long res;
	res = register_read(cpu, REG_HI);
	register_write(cpu, instr.rd, res);
	register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
	if(printable){
		printf("MFHI $%d -> 0x%s\n", instr.rd, instr.instrHex);
	}
}

void instructionMFLO(CPU *cpu, instruction instr, int printable){
	long res;
	res = register_read(cpu, REG_LO);
	register_write(cpu, instr.rd, res);
	register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
	if(printable){
		printf("MFLO $%d -> 0x%s\n", instr.rd, instr.instrHex);
	}
}

void instructionMULT(CPU *cpu, instruction instr, int printable){
	long resHI, resLO;
	resLO = (register_read(cpu, instr.rs) * register_read(cpu, instr.rt)) & 65535;
	resHI = register_read(cpu, instr.rs) * register_read(cpu, instr.rt) - resLO;
	register_write(cpu, REG_HI, resHI);
	register_write(cpu, REG_LO, resLO);
	register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
	if(printable){
		printf("MULT $%d,$%d -> 0x%s\n", instr.rs, instr.rt, instr.instrHex);
	}
}

void instructionNOP(CPU *cpu, instruction instr, int printable){
	register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
	if(printable){
		printf("NOP -> 0x%s\n", instr.instrHex);
	}
}

void instructionOR(CPU *cpu, instruction instr, int printable){
	long res;
	res = register_read(cpu, instr.rs) | register_read(cpu, instr.rt);
	register_write(cpu, instr.rd, res);
	register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
	if(printable){
		printf("OR $%d,$%d,$%d -> 0x%s\n", instr.rd, instr.rs, instr.rt, instr.instrHex);
	}
}

void instructionROTR(CPU *cpu, instruction instr, int printable){
	long d, res;
	d = register_read(cpu, instr.sa);
	res = register_read(cpu, instr.rt);
	res = res >> d | res << (32 - d);
	register_write(cpu, instr.rd, res);
	register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
	if(printable){
		printf("ROTR $%d,$%d,%d -> 0x%s\n", instr.rd, instr.rt, instr.sa, instr.instrHex);
	}
}

void instructionSLL(CPU *cpu, instruction instr, int printable){
	long nb, res;
	int i;
	nb = register_read(cpu, instr.sa);
	res = register_read(cpu, instr.rt);
	for(i = 0; i < nb; i++){
		res = (res * 2) & 8589934591;
	}
	register_write(cpu, instr.rd, res);
	register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
	if(printable){
		printf("SLL $%d,$%d,%d -> 0x%s\n", instr.rd, instr.rt, instr.sa, instr.instrHex);
	}
}

void instructionSLT(CPU *cpu, instruction instr, int printable){
	if(register_read(cpu, instr.rs) < register_read(cpu, instr.rt)){
		register_write(cpu, instr.rd, 1);
	}
	else{
		register_write(cpu, instr.rd, 0);
	}
	register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
	if(printable){
		printf("SLT $%d,$%d,$%d -> 0x%s\n", instr.rd, instr.rs, instr.rt, instr.instrHex);
	}
}

void instructionSRL(CPU *cpu, instruction instr, int printable){
	long nb, res;
	int i;
	nb = register_read(cpu, instr.sa);
	res = register_read(cpu, instr.rt);
	for(i = 0; i < nb; i++){
		res /= 2;
	}
	register_write(cpu, instr.rd, res);
	register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
	if(printable){
		printf("SRL $%d,$%d,%d -> 0x%s\n", instr.rd, instr.rt, instr.sa, instr.instrHex);
	}
}

void instructionSUB(CPU *cpu, instruction instr, int printable){
	long res;
	res = register_read(cpu, instr.rs) - register_read(cpu, instr.rt);
	register_write(cpu, instr.rd, res);
	register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
	if(printable){
		printf("SUB $%d,$%d,$%d -> 0x%s\n", instr.rd, instr.rs, instr.rt, instr.instrHex);
	}
}

void instructionSW(CPU *cpu, instruction instr, int printable){
	long res, mem;
	res = register_read(cpu, instr.rt);
	mem = register_read(cpu, instr.base) + instr.offset;
	memory_write(cpu, (int) mem, (int) res);
	register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
	if(printable){
		printf("SW $%d,%d($%d) -> 0x%s\n", instr.rt, instr.offset, instr.base, instr.instrHex);
	}
}

void instructionXOR(CPU *cpu, instruction instr, int printable){
	long res;
	res = register_read(cpu, instr.rs) ^ register_read(cpu, instr.rt);
	register_write(cpu, instr.rd, res);
	register_write(cpu, REG_PC, register_read(cpu, REG_PC) + 1);
	if(printable){
		printf("XOR $%d,$%d,$%d -> 0x%s\n", instr.rd, instr.rs, instr.rt, instr.instrHex);
	}
}