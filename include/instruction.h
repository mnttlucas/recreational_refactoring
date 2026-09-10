#ifndef	INSTRUCTION_H
#define	INSTRUCTION_H

enum OPCODE
{
	DUMMY_OPCODE,
	/* instruction object will be initialized to 0, 
	when line is fully skipped because of a comment, 
	OPCODE field will be equal to this dummy value for now */
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
	enum OPCODE opcode;
	int type;
	int etiq;
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

#endif	/* INSTRUCTION_H */