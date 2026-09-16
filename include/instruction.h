#ifndef	INSTRUCTION_H
#define	INSTRUCTION_H

/* MIPS bit fields */
#define OPCODE_START  0
#define OPCODE_END    5
#define RS_START      6
#define RS_END       10
#define RT_START     11
#define RT_END       15
#define RD_START     16
#define RD_END       20
#define SHAMT_START  21
#define SHAMT_END    25
#define FUNCT_START  26
#define FUNCT_END    31
#define IMM_START    16
#define IMM_END      31
#define TARGET_START  6
#define TARGET_END   31

/* Function codes */
#define FUNCT_ADD    32
#define FUNCT_ADDU   33
#define FUNCT_AND    36
#define FUNCT_CLO    17
#define FUNCT_CLZ    16
#define FUNCT_DIV    26
#define FUNCT_JR      8
#define FUNCT_MFHI   16
#define FUNCT_MFLO   18
#define FUNCT_MTHI   17
#define FUNCT_MTLO   19
#define FUNCT_MULT   24
#define FUNCT_NOR    39
#define FUNCT_OR     37
#define FUNCT_SLL     0
#define FUNCT_SLLV    4
#define FUNCT_SLT    42
#define FUNCT_SLTU   43
#define FUNCT_SRA     3
#define FUNCT_SRAV    7
#define FUNCT_SRL     2
#define FUNCT_SRLV    6
#define FUNCT_SUB    34
#define FUNCT_SUBU   35
#define FUNCT_XOR    38

/* Operation codes */
#define OPCODE_ADDI     8
#define OPCODE_ADDIU    9
#define OPCODE_ANDI    12
#define OPCODE_AUI     15
#define OPCODE_BEQ      4
#define OPCODE_BGTZ     7
#define OPCODE_BLEZ     6
#define OPCODE_BNE      5
#define OPCODE_J        2
#define OPCODE_JAL      3
#define OPCODE_LW      35
#define OPCODE_ORI     13
#define OPCODE_REGIMM   1
#define OPCODE_SLTI    10
#define OPCODE_SLTIU   11
#define OPCODE_SPECIAL  0
#define OPCODE_SW      43
#define OPCODE_XORI    14

/* Special bit locations */
#define ROTR_BIT    10
#define ROTRV_BIT   25

/* Special register values for some operations 
Even if some values are zeros, this exists for clarity and to match MIPS32 R6 specification */
#define BGEZ_RT    1
#define BGTZ_RT    0
#define BLTZ_RT    0
#define CLO_RT     0
#define CLZ_RT     0
#define CLO_SA     1
#define CLZ_SA     1

typedef enum
{
	OPCODE_MIN,
	/* instruction object will be initialized to 0, 
	when line is fully skipped because of a comment, 
	OPCODE field will be equal to this dummy value for now */
	ADD,
	ADDI,
	ADDIU,
	ADDU,
	AND,
	ANDI,
	AUI,
	BEQ,
	BGTZ,
	BLEZ,
	BNE,
	CLO,
	CLZ,
	DIV,
	J,
	JAL,
	JR,
	LW,
	MFHI,
	MFLO,
	MULT,
	MTHI,
	MTLO,
	NOP,
	NOR,
	OR,
	ORI,
	ROTR,
	ROTRV,
	SLL,
	SLLV,
	SLT,
	SLTI,
	SLTIU,
	SLTU,
	SRA,
	SRAV,
	SRL,
	SRLV,
	SUB,
	SUBU,
	SW,
	XOR,
	XORI,
	OPCODE_MAX
} opcode;

typedef enum
{
	BRANCH_R1,
	BRANCH_R2,
	CMD_EXIT,
	CMD_NOP,
	MEMORY,
	R2,
	R3_IMMEDIATE,
	R3,
	RD,
	RD_RS,
	RS,
	RT_IMMEDIATE,
	SHIFT,
	TARGET,
	VARIABLE_SHIFT
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
	int label;
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

const instruction_desc *find_instruction(const char *mnemonic);

#endif	/* INSTRUCTION_H */