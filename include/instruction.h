#ifndef	INSTRUCTION_H
#define	INSTRUCTION_H

#include <stdint.h>

/* MIPS bit fields */
#define OPCODE_START     0
#define OPCODE_END       5
#define BASE_START       RS_START
#define BASE_END         RS_END
#define RS_START         6
#define RS_END          10
#define RT_START        11
#define RT_END          15
#define RD_START        16
#define RD_END          20
#define SHAMT_START     21
#define SHAMT_END       25
#define FUNCT_START     26
#define FUNCT_END       31
#define OFFSET_26_START 6
#define OFFSET_21_START 11
#define OFFSET_16_START 16
#define OFFSET_END      31
#define IMM_START       16
#define IMM_END         31
#define TARGET_START     6
#define TARGET_END      31

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
#define FUNCT_SELEQZ 53
#define FUNCT_SELNEZ 55
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
#define OPCODE_BALC    58
#define OPCODE_BC      50
#define OPCODE_BEQ      4
#define OPCODE_BGTZ     7
#define OPCODE_BLEZ     6
#define OPCODE_BNE      5
#define OPCODE_J        2
#define OPCODE_JAL      3
#define OPCODE_LB      32
#define OPCODE_LBU     36
#define OPCODE_LH      33
#define OPCODE_LHU     37
#define OPCODE_LW      35
#define OPCODE_ORI     13
#define OPCODE_PCREL   59
#define OPCODE_POP06    6
#define OPCODE_POP07    7
#define OPCODE_POP10    8
#define OPCODE_POP26   22
#define OPCODE_POP27   23
#define OPCODE_POP30   24
#define OPCODE_POP66   54
#define OPCODE_POP76   62
#define OPCODE_REGIMM   1
#define OPCODE_SB      40
#define OPCODE_SH      41
#define OPCODE_SLTI    10
#define OPCODE_SLTIU   11
#define OPCODE_SPECIAL  0
#define OPCODE_SW      43
#define OPCODE_XORI    14

/* Offset sizes */
#define OFFSET_16_SIZE   16
#define OFFSET_21_SIZE   21
#define OFFSET_26_SIZE   26

/* Special bit locations */
#define ROTR_BIT    10
#define ROTRV_BIT   25

/* Special register values for some operations 
This exists for clarity and to match MIPS32 R6 specification
When values are zero, it will be implicit */
#define AUIPC_RT   30
#define BAL_RT     17
#define BGEZ_RT    1
#define CLO_CLZ_SA 1

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
	AUIPC,
	BAL,
	BALC,
	BC,
	BEQ,
	BEQC,
	BEQZC,
	BGEC,
	BGEUC,
	BGEZ,
	BGEZC,
	BGTZ,
	BGTZC,
	BLEZ,
	BLEZC,
	BLTC,
	BLTUC,
	BLTZ,
	BLTZC,
	BNE,
	BNEC,
	BNEZC,
	CLO,
	CLZ,
	DIV,
	J,
	JAL,
	JALR,
	JIALC,
	JIC,
	JR,
	LB,
	LBU,
	LH,
	LHU,
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
	SB,
	SELEQZ,
	SELNEZ,
	SH,
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
} Opcode;

typedef enum
{
	CMD_EXIT,
	CMD_NOP,
	OFFSET_16,
	OFFSET_26,
	RD,
	RD_RS,
	RD_RS_RT,
	RD_RT_RS,
	RD_RT_SA,
	RS,
	RS_IMMEDIATE,
	RS_OFFSET_16,
	RS_OFFSET_21,
	RS_RT,
	RS_RT_OFFSET_16,
	RT_IMMEDIATE,
	RT_OFFSET_16,
	RT_OFFSET_BASE,
	RT_RS_IMMEDIATE,
	RT_RS_OFFSET_16,
	TARGET
} format;

typedef struct
{
	const char *mnemonic;
	format format;
	int code;
	Opcode op;
} instruction_desc;

typedef struct
{
	char instr_hex[9];
	char to_string[200];
	int label;
	int target;
	int16_t immediate;
	int32_t offset;
	Opcode opcode;
	uint8_t base;
	uint8_t exit;
	uint8_t instr_bin[32];
	uint8_t negative;
	uint8_t rd;
	uint8_t rs;
	uint8_t rt;
	uint8_t sa;
} instruction;

const instruction_desc *find_instruction(const char *mnemonic);

#endif	/* INSTRUCTION_H */