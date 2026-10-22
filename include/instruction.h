#ifndef	INSTRUCTION_H
#define	INSTRUCTION_H

#include <stdint.h>

/* Errors : bit flags, several can be set on the same instruction (use |=, test with &) */
#define NO_ERR                      0u
#define ERR_INVALID_REGISTER        (1u << 0)
#define ERR_INVALID_NUMBER          (1u << 1)
#define ERR_RANGE                   (1u << 2)
#define ERR_SYNTAX                  (1u << 3)
#define ERR_UNKNOWN_INSTRUCTION     (1u << 4)
#define ERR_CONSTRAINT              (1u << 5)

typedef enum
{
	OP_MIN,
	/* instruction object will be initialized to 0,
	when line is fully skipped because of a comment,
	opcode field will be equal to this dummy value for now */
	OP_ABS_D,
	OP_ABS_S,
	OP_ADD,
	OP_ADDIU,
	OP_ADDIUPC,
	OP_ADDU,
	OP_ADD_D,
	OP_ADD_S,
	OP_ALUIPC,
	OP_AND,
	OP_ANDI,
	OP_AUI,
	OP_AUIPC,
	OP_BAL,
	OP_BALC,
	OP_BC,
	OP_BC1EQZ,
	OP_BC1NEZ,
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
	OP_CEIL_L_D,
	OP_CEIL_L_S,
	OP_CEIL_W_D,
	OP_CEIL_W_S,
	OP_CLASS_D,
	OP_CLASS_S,
	OP_CLO,
	OP_CLZ,
	OP_CMP_D,
	OP_CMP_S,
	OP_CRC32B,
	OP_CRC32CB,
	OP_CRC32CH,
	OP_CRC32CW,
	OP_CRC32H,
	OP_CRC32W,
	OP_CVT_D_L,
	OP_CVT_D_S,
	OP_CVT_D_W,
	OP_CVT_L_D,
	OP_CVT_L_S,
	OP_CVT_S_D,
	OP_CVT_S_L,
	OP_CVT_S_W,
	OP_CVT_W_D,
	OP_CVT_W_S,
	OP_DIV,
	OP_DIVU,
	OP_DIV_D,
	OP_DIV_S,
	OP_FLOOR_L_D,
	OP_FLOOR_L_S,
	OP_FLOOR_W_D,
	OP_FLOOR_W_S,
	OP_J,
	OP_JAL,
	OP_JALR,
	OP_JIALC,
	OP_JIC,
	OP_LB,
	OP_LBU,
	OP_LDC1,
	OP_LH,
	OP_LHU,
	OP_LW,
	OP_LWC1,
	OP_LWPC,
	OP_MADDF_D,
	OP_MADDF_S,
	OP_MAX_D,
	OP_MAX_S,
	OP_MAXA_D,
	OP_MAXA_S,
	OP_MFC1,
	OP_MFHC1,
	OP_MIN_D,
	OP_MIN_S,
	OP_MINA_D,
	OP_MINA_S,
	OP_MOD,
	OP_MODU,
	OP_MOV_D,
	OP_MOV_S,
	OP_MSUBF_D,
	OP_MSUBF_S,
	OP_MTC1,
	OP_MTHC1,
	OP_MUH,
	OP_MUHU,
	OP_MUL,
	OP_MULU,
	OP_MUL_D,
	OP_MUL_S,
	OP_NAL,
	OP_NEG_D,
	OP_NEG_S,
	OP_NOP,
	OP_NOR,
	OP_OR,
	OP_ORI,
	OP_RECIP_D,
	OP_RECIP_S,
	OP_RINT_D,
	OP_RINT_S,
	OP_ROTR,
	OP_ROTRV,
	OP_ROUND_L_D,
	OP_ROUND_L_S,
	OP_ROUND_W_D,
	OP_ROUND_W_S,
	OP_RSQRT_D,
	OP_RSQRT_S,
	OP_SB,
	OP_SDC1,
	OP_SEB,
	OP_SEH,
	OP_SEL_D,
	OP_SEL_S,
	OP_SELEQZ,
	OP_SELEQZ_D,
	OP_SELEQZ_S,
	OP_SELNEZ,
	OP_SELNEZ_D,
	OP_SELNEZ_S,
	OP_SH,
	OP_SLL,
	OP_SLLV,
	OP_SLT,
	OP_SLTI,
	OP_SLTIU,
	OP_SLTU,
	OP_SQRT_D,
	OP_SQRT_S,
	OP_SRA,
	OP_SRAV,
	OP_SRL,
	OP_SRLV,
	OP_SUB,
	OP_SUBU,
	OP_SUB_D,
	OP_SUB_S,
	OP_SW,
	OP_SWC1,
	OP_TRUNC_L_D,
	OP_TRUNC_L_S,
	OP_TRUNC_W_D,
	OP_TRUNC_W_S,
	OP_XOR,
	OP_XORI,
	OP_MAX
} Opcode;

typedef enum
{
	FMT_EXIT,
	FMT_NAL,
	FMT_NOP,
	FMT_FD_FS,
	FMT_FD_FS_FT,
	FMT_FT_OFFSET_16,
	FMT_FT_OFFSET_16_BASE,
	FMT_OFFSET_16,
	FMT_OFFSET_26,
	FMT_RD_RS,
	FMT_RD_RT,
	FMT_RD_RS_RT,
	FMT_RD_RT_RS,
	FMT_RD_RT_SA,
	FMT_RS,
	FMT_RS_IMMEDIATE_16,
	FMT_RS_IMMEDIATE_19,
	FMT_RS_OFFSET_16,
	FMT_RS_OFFSET_19,
	FMT_RS_OFFSET_21,
	FMT_RS_RT,
	FMT_RS_RT_OFFSET_16,
	FMT_RT_FS,
	FMT_RT_IMMEDIATE_16,
	FMT_RT_OFFSET_16,
	FMT_RT_OFFSET_BASE,
	FMT_RT_RS_IMMEDIATE_16,
	FMT_RT_RS_OFFSET_16,
	FMT_RT_RS_RT,
	FMT_TARGET
} Format;

typedef struct
{
	const char *mnemonic;
	Format format;
	int code;
	Opcode op;
	int fmt;	/* COP1 `fmt` field (rs position), 0 for the other instructions. For CVT, the format of the source operand */
} InstructionDesc;

typedef struct
{
	char to_string[256];
	int32_t immediate;
	int32_t offset;
	int32_t target;
	Opcode opcode;
	uint32_t encoding;
	uint8_t base;
	uint8_t error;
	uint8_t exit;
	uint8_t fd, fs, ft;
	uint8_t rd, rs, rt;
	uint8_t sa;
} Instruction;

const InstructionDesc *find_instruction(const char *mnemonic);

#endif	/* INSTRUCTION_H */
