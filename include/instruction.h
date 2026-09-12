#ifndef	INSTRUCTION_H
#define	INSTRUCTION_H

#define OPCODE_START   0
#define OPCODE_END     5
#define RS_START       6
#define RS_END        10
#define RT_START      11
#define RT_END        15
#define RD_START      16
#define RD_END        20
#define SHAMT_START   21
#define SHAMT_END     25
#define FUNCT_START   26
#define FUNCT_END     31
#define IMM_START     16
#define IMM_END       31
#define TARGET_START   6
#define TARGET_END    31

#define FUNCT_ADD    32
#define FUNCT_SUB    34
#define FUNCT_AND    36
#define FUNCT_OR     37
#define FUNCT_XOR    38
#define FUNCT_SLT    42
#define FUNCT_MULT   24
#define FUNCT_DIV    26
#define FUNCT_MFHI   16
#define FUNCT_MFLO   18
#define FUNCT_JR      8
#define FUNCT_SLL     0
#define FUNCT_SRL     2

#define OPCODE_J      2
#define OPCODE_JAL    3
#define OPCODE_BEQ    4
#define OPCODE_BNE    5
#define OPCODE_BLEZ   6
#define OPCODE_BGTZ   7
#define OPCODE_ADDI   8
#define OPCODE_LUI   15
#define OPCODE_LW    35
#define OPCODE_SW    43

#define ROTR_BIT 10

typedef enum
{
	OPCODE_MIN,
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
	XOR,
	OPCODE_MAX
} opcode;

typedef enum
{
	BRANCH_R1,
	BRANCH_R2,
	MEMORY,
	R2,
	R3_IMMEDIATE,
	R3,
	RD,
	RS,
	SHIFT,
	TARGET
} format;

typedef struct
{
	const char *mnemonic;
	opcode op;
	int code;
	format format;
} instruction_desc;

typedef struct
{
	int instr_bin[32];
	char instr_hex[9];
	char to_string[200];
	int etiq;
	int rs;
	int rt;
	int rd;
	int sa;
	int offset;
	int base;
	int immediate;
	int negative;
	int target;
	int exit;
	opcode opcode;
} instruction;

#endif	/* INSTRUCTION_H */