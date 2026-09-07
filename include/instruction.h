#ifndef INSTRUCTION_H
#define INSTRUCTION_H

enum OPCODE {
	ADD,
	ADDI,
	AND,
	BEQ,
	BGTZ,
	BLEZ,
	BNE,
	DIV,
	J,
	JAL,
	JR,
	LUI,
	LW,
	MFHI,
	MFLO,
	MULT,
	NOP,
	OR,
	ROTR,
	SLL,
	SLT,
	SRL,
	SUB,
	SW,
	XOR
};

typedef struct instruction
{
	int type;          	
	int etiq;
	enum OPCODE opcode;
	int rs;
	int rt;
	int rd;
	int sa;
	int offset;
	int base;
	int immediate;
	int target;
	char instrHex[9];
	char toString[200];
	int exit;
} instruction;

#endif /* INSTRUCTION_H */