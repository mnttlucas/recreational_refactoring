#ifndef INSTRUCTION_H
#define INSTRUCTION_H

typedef struct instruction{
	int type;          	
	int etiq;
	int opcode;
	int rs;
	int rt;
	int rd;
	int sa;
	int offset;
	int base;
	int immediate;
	int target;
	char instrHex[9];
	int exit;
} instruction;

#endif /* INSTRUCTION_H */