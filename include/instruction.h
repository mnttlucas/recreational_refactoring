#ifndef	INSTRUCTION_H
#define	INSTRUCTION_H

#include <stdint.h>

/* Errors : bit flags, several can be set on the same instruction (use |=, test with &) */
#define NO_ERR                      0
#define ERR_INVALID_REGISTER        (1u << 0)
#define ERR_INVALID_NUMBER          (1u << 1)
#define ERR_RANGE                   (1u << 2)
#define ERR_SYNTAX                  (1u << 3)
#define ERR_UNKNOWN_INSTRUCTION     (1u << 4)

/* MIPS bit fields : position of the least significant bit of each field in the 32-bit encoding */
#define FUNCT_SHIFT      0
#define SHAMT_SHIFT      6
#define RD_SHIFT        11
#define RT_SHIFT        16
#define RS_SHIFT        21
#define BASE_SHIFT      RS_SHIFT
#define OPCODE_SHIFT    26

/* Function codes */
#define FUNCT_ADD    32
#define FUNCT_ADDU   33
#define FUNCT_AND    36
#define FUNCT_BSHFL  32
#define FUNCT_CLO    17
#define FUNCT_CLZ    16
#define FUNCT_JALR    9
#define FUNCT_JR      8
#define FUNCT_SOP30  24
#define FUNCT_SOP31  25
#define FUNCT_SOP32  26
#define FUNCT_SOP33  27
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
#define OPCODE_ADDIU      9
#define OPCODE_ANDI      12
#define OPCODE_AUI       15
#define OPCODE_BALC      58
#define OPCODE_BC        50
#define OPCODE_BEQ        4
#define OPCODE_BGTZ       7
#define OPCODE_BLEZ       6
#define OPCODE_BNE        5
#define OPCODE_J          2
#define OPCODE_JAL        3
#define OPCODE_LB        32
#define OPCODE_LBU       36
#define OPCODE_LH        33
#define OPCODE_LHU       37
#define OPCODE_LW        35
#define OPCODE_ORI       13
#define OPCODE_PCREL     59
#define OPCODE_POP06      6
#define OPCODE_POP07      7
#define OPCODE_POP10      8
#define OPCODE_POP26     22
#define OPCODE_POP27     23
#define OPCODE_POP30     24
#define OPCODE_POP66     54
#define OPCODE_POP76     62
#define OPCODE_REGIMM     1
#define OPCODE_SB        40
#define OPCODE_SH        41
#define OPCODE_SLTI      10
#define OPCODE_SLTIU     11
#define OPCODE_SPECIAL    0
#define OPCODE_SPECIAL3  31
#define OPCODE_SW        43
#define OPCODE_XORI      14

/* Offset and immediate sizes */
#define IMM_16_SIZE      16
#define OFFSET_16_SIZE   16
#define IMM_19_SIZE      19
#define OFFSET_19_SIZE   19
#define OFFSET_21_SIZE   21
#define OFFSET_26_SIZE   26

/* Special register values for some operations 
This exists for clarity and to match MIPS32 R6 specification
When values are zero, it will be implicit */
#define ALUIPC_RT     31
#define LWPC_RT        8
#define ROTR_RS        1
#define ROTRV_SA       1
#define AUIPC_RT      30
#define BAL_RT        17
#define BGEZ_RT        1
#define CLO_CLZ_SA     1
#define DIV_DIVU_SA    2
#define MOD_MODU_SA    3
#define MUH_MUHU_SA    3
#define MUL_MULU_SA    2
#define NAL_RT        16
#define SEB_SA        16
#define SEH_SA        24

typedef enum
{
	OP_MIN,
	/* instruction object will be initialized to 0, 
	when line is fully skipped because of a comment, 
	OPCODE field will be equal to this dummy value for now */
	OP_ADD,
	OP_ADDIU,
	OP_ADDIUPC,
	OP_ADDU,
	OP_ALUIPC,
	OP_AND,
	OP_ANDI,
	OP_AUI,
	OP_AUIPC,
	OP_BAL,
	OP_BALC,
	OP_BC,
	OP_BEQ,
	OP_BEQC,
	OP_BEQZALC,
	OP_BEQZC,
	OP_BGEC,
	OP_BGEUC,
	OP_BGEZ,
	OP_BGEZALC,
	OP_BGEZC,
	OP_BGTZ,
	OP_BGTZALC,
	OP_BGTZC,
	OP_BITSWAP,
	OP_BLEZ,
	OP_BLEZALC,
	OP_BLEZC,
	OP_BLTC,
	OP_BLTUC,
	OP_BLTZ,
	OP_BLTZALC,
	OP_BLTZC,
	OP_BNE,
	OP_BNEC,
	OP_BNEZALC,
	OP_BNEZC,
	OP_BNVC,
	OP_BOVC,
	OP_CLO,
	OP_CLZ,
	OP_DIV,
	OP_DIVU,
	OP_J,
	OP_JAL,
	OP_JALR,
	OP_JIALC,
	OP_JIC,
	OP_JR,
	OP_LB,
	OP_LBU,
	OP_LH,
	OP_LHU,
	OP_LW,
	OP_LWPC,
	OP_MOD,
	OP_MODU,
	OP_MUH,
	OP_MUHU,
	OP_MUL,
	OP_MULU,
	OP_NAL,
	OP_NOP,
	OP_NOR,
	OP_OR,
	OP_ORI,
	OP_ROTR,
	OP_ROTRV,
	OP_SB,
	OP_SEB,
	OP_SEH,
	OP_SELEQZ,
	OP_SELNEZ,
	OP_SH,
	OP_SLL,
	OP_SLLV,
	OP_SLT,
	OP_SLTI,
	OP_SLTIU,
	OP_SLTU,
	OP_SRA,
	OP_SRAV,
	OP_SRL,
	OP_SRLV,
	OP_SUB,
	OP_SUBU,
	OP_SW,
	OP_XOR,
	OP_XORI,
	OP_MAX
} Opcode;

typedef enum
{
	CMD_EXIT,
	CMD_NAL,
	CMD_NOP,
	FMT_OFFSET_16,
	FMT_OFFSET_26,
	FMT_RD, // Previously used by MFHI/MFLO instructions, unused for now, will remove it if no instruction use it
	FMT_RD_RS,
	FMT_RD_RT,
	FMT_RD_RS_RT,
	FMT_RD_RT_RS,
	FMT_RD_RT_SA,
	FMT_RS, // Previously used by MTHI/MTLO instructions, unused for now, will remove it if no instruction use it
	FMT_RS_IMMEDIATE_16,
	FMT_RS_IMMEDIATE_19,
	FMT_RS_OFFSET_16,
	FMT_RS_OFFSET_19,
	FMT_RS_OFFSET_21,
	FMT_RS_RT, // Previously used by OP_DIV/OP_MUL instructions, unused for now, will remove it if no instruction use it
	FMT_RS_RT_OFFSET_16,
	FMT_RT_IMMEDIATE_16,
	FMT_RT_OFFSET_16,
	FMT_RT_OFFSET_BASE,
	FMT_RT_RS_IMMEDIATE_16,
	FMT_RT_RS_OFFSET_16,
	FMT_TARGET
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
	char to_string[256];
	int label;
	int target;
	int32_t immediate;
	int32_t offset;
	Opcode opcode;
	uint32_t encoding;
	uint8_t base;
	uint8_t error;
	uint8_t exit;
	uint8_t fd, fs, ft;
	uint8_t rd, rs, rt;
	uint8_t sa;
} instruction;

const instruction_desc *find_instruction(const char *mnemonic);

#endif	/* INSTRUCTION_H */