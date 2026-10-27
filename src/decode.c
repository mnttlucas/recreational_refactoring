#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decode.h"
#include "display.h"
#include "hex.h"
#include "instruction.h"
#include "mips_encoding.h"
#include "mips_registers.h"
#include "utils.h"

#define ALIGN_BP_MAX 3
#define LSA_SA_MAX   4
#define SA_MAX      31
#define TARGET_MAX ((INT32_C(1) << OFFSET_26_SIZE) - 1)

/* Error messages of the assembler. They are muted while a hexadecimal word is decoded : many table entries are tried
   on the same word and most of them are rejected, which is not an error. */
static int quiet;

static void report(const char *format, ...)
{
	va_list args;

	if(quiet) return;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}

void decode_set_quiet(int mute)
{
	quiet = mute;
}

static void constraint_error(Instruction *instr, const char *rule)
{
	report("[!] Register constraint : %s\n", rule);
	instr->error |= ERR_CONSTRAINT;
}

static void syntax_error(Instruction *instr)
{
	report("[!] Syntax error : missing or malformed operands\n");
	instr->error |= ERR_SYNTAX;
}

/* Parses a decimal integer (optional sign, optional trailing spaces).
   The whole token must be consumed, otherwise the instruction is rejected. */
static int32_t parse_integer(const char *str, Instruction *instr)
{
	char *end;
	long value;

	errno = 0;
	value = strtol(str, &end, 10);

	if(end == str)
	{
		report("[!] Invalid number : '%s'\n", str);
		instr->error |= ERR_INVALID_NUMBER;
		return(0);
	}

	while(isspace((unsigned char) *end)) end++;

	if(*end != '\0')
	{
		report("[!] Invalid number : '%s'\n", str);
		instr->error |= ERR_INVALID_NUMBER;
		return(0);
	}

	if(errno == ERANGE || value > INT32_MAX || value < INT32_MIN)
	{
		report("[!] Number out of range : '%s'\n", str);
		instr->error |= ERR_RANGE;
		return(0);
	}

	return((int32_t) value);
}

/* Which values an immediate or an offset field accepts, as the manual defines the field :
   - RANGE_SIGNED   : two's complement, [-2^(size-1); 2^(size-1) - 1] (offsets, ADDIU, SLTI, SLTIU...)
   - RANGE_UNSIGNED : zero-extended, [0; 2^size - 1] (ANDI, ORI, XORI)
   - RANGE_PATTERN  : a 16-bit pattern placed in the upper half, both 0xFFFF and -1 are the same field (LUI, AUI, AUIPC, ALUIPC) */
Range decode_immediate_range(Opcode opcode)
{
	switch(opcode)
	{
		case OP_ANDI :
		case OP_ORI :
		case OP_XORI :
			return(RANGE_UNSIGNED);
		case OP_AUI :
		case OP_AUIPC :
		case OP_ALUIPC :
			return(RANGE_PATTERN);
		default :
			return(RANGE_SIGNED);
	}
}

/* Returns the value sign-extended, as the execution expects, or 0 with ERR_RANGE if it is out of range */
static int32_t check_field(Instruction *instr, int32_t value, uint8_t size, Range range)
{
	const int32_t half = INT32_C(1) << (size - 1), full = (INT32_C(1) << size) - 1;
	const int32_t min = range == RANGE_UNSIGNED ? 0 : -half;
	const int32_t max = range == RANGE_SIGNED ? half - 1 : full;

	if(value < min || value > max)
	{
		report("[!] Value out of range : %" PRId32 " does not fit in %d bits (allowed : %" PRId32 " to %" PRId32 ")\n", value, size, min, max);
		instr->error |= ERR_RANGE;
		return(0);
	}

	return(sign_extend(value, size));
}

static void swap_rs_rt(Instruction *instr)
{
	const uint8_t tmp = instr->rs;
	instr->rs = instr->rt;
	instr->rt = tmp;
}

/* The R6 compact branches share their opcode (POP06, POP07, POP10, POP26, POP27, POP30, POP66, POP76) and are
   told apart by their registers. An encoding that breaks the rule of its instruction would be another instruction.
   BEQC / BNEC (rs < rt) and BOVC / BNVC (rs >= rt) are symmetric, their operands are swapped when needed. */
static void check_compact_branch(Instruction *instr)
{
	if(instr->error != NO_ERR) return;

	switch(instr->opcode)
	{
		case OP_BEQC :
		case OP_BNEC :
			if(instr->rs > instr->rt) swap_rs_rt(instr);
			if(instr->rs == instr->rt) constraint_error(instr, "rs and rt must be different");
			else if(instr->rs == REG_ZERO) constraint_error(instr, "rs and rt must not be $zero (use BEQZC / BNEZC)");
			break;
		case OP_BOVC :
		case OP_BNVC :
			if(instr->rs < instr->rt) swap_rs_rt(instr);
			break;
		case OP_BGEC :
		case OP_BGEUC :
		case OP_BLTC :
		case OP_BLTUC :
			if(instr->rs == instr->rt) constraint_error(instr, "rs and rt must be different");
			else if(instr->rs == REG_ZERO || instr->rt == REG_ZERO) constraint_error(instr, "rs and rt must not be $zero");
			break;
		case OP_BEQZALC :
		case OP_BGEZALC :
		case OP_BGEZC :
		case OP_BGTZALC :
		case OP_BGTZC :
		case OP_BLEZALC :
		case OP_BLEZC :
		case OP_BLTZALC :
		case OP_BLTZC :
		case OP_BNEZALC :
			if(instr->rt == REG_ZERO) constraint_error(instr, "rt must not be $zero");
			break;
		case OP_BEQZC :
		case OP_BNEZC :
			if(instr->rs == REG_ZERO) constraint_error(instr, "rs must not be $zero (this encoding is JIC / JIALC)");
			break;
		default :
			break;
	}
}

static uint32_t field(int32_t value, uint8_t shift, uint8_t size)
{
	return(((uint32_t) value & low_mask(size)) << shift);
}

/* `value` is the already checked and sign-extended immediate/offset */
static uint32_t encode_i(uint32_t op, uint32_t rs, uint32_t rt, int32_t value, uint8_t size)
{
	return((op << OPCODE_SHIFT) | (rs << RS_SHIFT) | (rt << RT_SHIFT) | field(value, 0, size));
}

static uint32_t encode_r(uint32_t op, uint32_t rs, uint32_t rt, uint32_t rd, uint32_t shamt, uint32_t funct)
{
	return((op << OPCODE_SHIFT) | (rs << RS_SHIFT) | (rt << RT_SHIFT) | (rd << RD_SHIFT) | (shamt << SHAMT_SHIFT) | (funct << FUNCT_SHIFT));
}

static void build_cmd_instruction(Instruction *instr, int funct_code, int op_code, int rs_value, int rt_value, int rd_value, int shamt_value)
{
	instr->encoding = encode_r((uint32_t) op_code, (uint32_t) rs_value, (uint32_t) rt_value, (uint32_t) rd_value, (uint32_t) shamt_value, (uint32_t) funct_code);
}

static void build_fd_fs_instruction(Instruction *instr, int funct_code, int fmt)
{
	instr->encoding = encode_r(OPCODE_COP1, (uint32_t) fmt, 0, instr->fs, instr->fd, (uint32_t) funct_code);
}

static void build_fd_fs_ft_instruction(Instruction *instr, int funct_code, int fmt)
{
	instr->encoding = encode_r(OPCODE_COP1, (uint32_t) fmt, instr->ft, instr->fs, instr->fd, (uint32_t) funct_code);
}

static void build_ft_offset_16_instruction(Instruction *instr, int op_code)
{
	uint32_t fs_value = 0;
	if(instr->opcode == OP_BC1EQZ) fs_value = BC1EQZ_FS;
	else if(instr->opcode == OP_BC1NEZ) fs_value = BC1NEZ_FS;
	instr->offset = check_field(instr, instr->offset, OFFSET_16_SIZE, RANGE_SIGNED);
	instr->encoding = encode_i((uint32_t) op_code, fs_value, instr->ft, instr->offset, OFFSET_16_SIZE);
}

static void build_ft_offset_16_base_instruction(Instruction *instr, int op_code)
{
	instr->offset = check_field(instr, instr->offset, OFFSET_16_SIZE, RANGE_SIGNED);
	instr->encoding = encode_i((uint32_t) op_code, instr->base, instr->ft, instr->offset, OFFSET_16_SIZE);
}

static void build_offset_16_instruction(Instruction *instr, int op_code)
{
	uint32_t rt_value = 0;
	if(instr->opcode == OP_BAL) rt_value = BAL_RT;
	instr->offset = check_field(instr, instr->offset, OFFSET_16_SIZE, RANGE_SIGNED);
	instr->encoding = encode_i((uint32_t) op_code, 0, rt_value, instr->offset, OFFSET_16_SIZE);
}

static void build_offset_26_instruction(Instruction *instr, int op_code)
{
	instr->offset = check_field(instr, instr->offset, OFFSET_26_SIZE, RANGE_SIGNED);
	instr->encoding = ((uint32_t) op_code << OPCODE_SHIFT) | field(instr->offset, 0, OFFSET_26_SIZE);
}

static void build_rd_rs_instruction(Instruction *instr, int funct_code)
{
	uint32_t shamt_value = 0;
	if(instr->opcode == OP_JALR && instr->error == NO_ERR)
	{
		if(instr->rd == REG_ZERO) constraint_error(instr, "rd must not be $zero (use JR)");
		/* Jump-and-Link Restartability (manual) : the instruction could not be re-executed after an exception in its delay slot */
		else if(instr->rd == instr->rs) constraint_error(instr, "rd and rs must be different (UNPREDICTABLE otherwise)");
	}
	if(instr->opcode == OP_CLO || instr->opcode == OP_CLZ) shamt_value = CLO_CLZ_SA;
	instr->encoding = encode_r(OPCODE_SPECIAL, instr->rs, 0, instr->rd, shamt_value, (uint32_t) funct_code);
}

static void build_rd_rs_rt_instruction(Instruction *instr, int funct_code)
{
	uint32_t shamt_value = 0;
	if(instr->opcode == OP_DIV || instr->opcode == OP_DIVU) shamt_value = DIV_DIVU_SA;
	else if(instr->opcode == OP_MOD || instr->opcode == OP_MODU) shamt_value = MOD_MODU_SA;
	else if(instr->opcode == OP_MUH || instr->opcode == OP_MUHU) shamt_value = MUH_MUHU_SA;
	else if(instr->opcode == OP_MUL || instr->opcode == OP_MULU) shamt_value = MUL_MULU_SA;
	instr->encoding = encode_r(OPCODE_SPECIAL, instr->rs, instr->rt, instr->rd, shamt_value, (uint32_t) funct_code);
}

static void build_rd_rs_rt_bp_instruction(Instruction *instr, int funct_code)
{
	instr->encoding = encode_r(OPCODE_SPECIAL3, instr->rs, instr->rt, instr->rd, ALIGN_SA | instr->sa, (uint32_t) funct_code);
}

/* The 2-bit sa field holds the scale - 1 (the shift is 1 to 4) */
static void build_rd_rs_rt_sa_instruction(Instruction *instr, int funct_code)
{
	instr->encoding = encode_r(OPCODE_SPECIAL, instr->rs, instr->rt, instr->rd, (uint32_t) (instr->sa - 1), (uint32_t) funct_code);
}

static void build_rd_rt_instruction(Instruction *instr, int funct_code)
{
	uint32_t shamt_value = 0;
	if(instr->opcode == OP_SEB) shamt_value = SEB_SA;
	else if(instr->opcode == OP_SEH) shamt_value = SEH_SA;
	else if(instr->opcode == OP_WSBH) shamt_value = WSBH_SA;
	instr->encoding = encode_r(OPCODE_SPECIAL3, 0, instr->rt, instr->rd, shamt_value, (uint32_t) funct_code);
}

static void build_rd_rt_rs_instruction(Instruction *instr, int funct_code)
{
	uint32_t shamt_value = 0;
	if(instr->opcode == OP_ROTRV) shamt_value = ROTRV_SA;
	instr->encoding = encode_r(OPCODE_SPECIAL, instr->rs, instr->rt, instr->rd, shamt_value, (uint32_t) funct_code);
}

static void build_rd_rt_sa_instruction(Instruction *instr, int funct_code)
{
	uint32_t rs_value = 0;
	if(instr->opcode == OP_ROTR) rs_value = ROTR_RS;
	instr->encoding = encode_r(OPCODE_SPECIAL, rs_value, instr->rt, instr->rd, instr->sa, (uint32_t) funct_code);
}

static void build_rs_instruction(Instruction *instr, int funct_code)
{
	instr->encoding = encode_r(OPCODE_SPECIAL, instr->rs, 0, 0, 0, (uint32_t) funct_code);
}

static void build_rs_immediate_16_instruction(Instruction *instr, int op_code)
{
	uint32_t rt_value = 0;
	if(instr->opcode == OP_AUIPC) rt_value = AUIPC_RT;
	else if(instr->opcode == OP_ALUIPC) rt_value = ALUIPC_RT;
	instr->immediate = check_field(instr, instr->immediate, IMM_16_SIZE, decode_immediate_range(instr->opcode));
	instr->encoding = encode_i((uint32_t) op_code, instr->rs, rt_value, instr->immediate, IMM_16_SIZE);
}

static void build_rs_immediate_19_instruction(Instruction *instr, int op_code)
{
	instr->immediate = check_field(instr, instr->immediate, IMM_19_SIZE, RANGE_SIGNED);
	instr->encoding = encode_i((uint32_t) op_code, instr->rs, 0, instr->immediate, IMM_19_SIZE);
}

static void build_rs_offset_16_instruction(Instruction *instr, int op_code)
{
	uint32_t rt_value = 0;
	if(instr->opcode == OP_BGEZ) rt_value = BGEZ_RT;
	instr->offset = check_field(instr, instr->offset, OFFSET_16_SIZE, RANGE_SIGNED);
	instr->encoding = encode_i((uint32_t) op_code, instr->rs, rt_value, instr->offset, OFFSET_16_SIZE);
}

static void build_rs_offset_19_instruction(Instruction *instr, int op_code)
{
	uint32_t rt_value = 0;
	if(instr->opcode == OP_LWPC) rt_value = LWPC_RT;
	instr->offset = check_field(instr, instr->offset, OFFSET_19_SIZE, RANGE_SIGNED);
	instr->encoding = encode_i((uint32_t) op_code, instr->rs, rt_value, instr->offset, OFFSET_19_SIZE);
}

static void build_rs_offset_21_instruction(Instruction *instr, int op_code)
{
	check_compact_branch(instr);
	instr->offset = check_field(instr, instr->offset, OFFSET_21_SIZE, RANGE_SIGNED);
	instr->encoding = encode_i((uint32_t) op_code, instr->rs, 0, instr->offset, OFFSET_21_SIZE);
}

static void build_rs_rt_instruction(Instruction *instr, int funct_code)
{
	instr->encoding = encode_r(OPCODE_SPECIAL, instr->rs, instr->rt, 0, 0, (uint32_t) funct_code);
}

static void build_rs_rt_offset_16_instruction(Instruction *instr, int op_code)
{
	check_compact_branch(instr);
	instr->offset = check_field(instr, instr->offset, OFFSET_16_SIZE, RANGE_SIGNED);
	instr->encoding = encode_i((uint32_t) op_code, instr->rs, instr->rt, instr->offset, OFFSET_16_SIZE);
}

static void build_rt_fs_instruction(Instruction *instr, int sub_code)
{
	instr->encoding = encode_r(OPCODE_COP1, (uint32_t) sub_code, instr->rt, instr->fs, 0, 0);
}

static void build_rt_immediate_16_instruction(Instruction *instr, int op_code)
{
	instr->immediate = check_field(instr, instr->immediate, IMM_16_SIZE, decode_immediate_range(instr->opcode));
	instr->encoding = encode_i((uint32_t) op_code, 0, instr->rt, instr->immediate, IMM_16_SIZE);
}

static void build_rt_offset_16_instruction(Instruction *instr, int op_code)
{
	uint32_t rs_value = 0;
	check_compact_branch(instr);
	/* These four share their opcode with BLEZALC, BGTZALC, BLEZC and BGTZC (rs = 0) : rs = rt tells them apart */
	if(instr->opcode == OP_BGEZALC || instr->opcode == OP_BLTZALC || instr->opcode == OP_BGEZC || instr->opcode == OP_BLTZC) rs_value = instr->rt;
	instr->offset = check_field(instr, instr->offset, OFFSET_16_SIZE, RANGE_SIGNED);
	instr->encoding = encode_i((uint32_t) op_code, rs_value, instr->rt, instr->offset, OFFSET_16_SIZE);
}

static void build_rt_offset_16_base_instruction(Instruction *instr, int op_code)
{
	instr->offset = check_field(instr, instr->offset, OFFSET_16_SIZE, RANGE_SIGNED);
	instr->encoding = encode_i((uint32_t) op_code, instr->base, instr->rt, instr->offset, OFFSET_16_SIZE);
}

static void build_rt_rs_immediate_16_instruction(Instruction *instr, int op_code)
{
	instr->immediate = check_field(instr, instr->immediate, IMM_16_SIZE, decode_immediate_range(instr->opcode));
	instr->encoding = encode_i((uint32_t) op_code, instr->rs, instr->rt, instr->immediate, IMM_16_SIZE);
}

/* EXT : msbd = size - 1, lsb = pos. INS : msb = pos + size - 1, lsb = pos. msb(d) is in the rd position, lsb in sa */
static void build_rt_rs_pos_size_instruction(Instruction *instr, int funct_code)
{
	const uint32_t msb = (instr->opcode == OP_INS) ? (uint32_t) (instr->pos + instr->size - 1) : (uint32_t) (instr->size - 1);
	instr->encoding = encode_r(OPCODE_SPECIAL3, instr->rs, instr->rt, msb, instr->pos, (uint32_t) funct_code);
}

static void build_rt_rs_rt_instruction(Instruction *instr, int funct_code)
{
	uint32_t shamt_value;
	switch(instr->opcode)
	{
		case OP_CRC32B :
			shamt_value = CRC32_SA_B;
			break;
		case OP_CRC32H :
			shamt_value = CRC32_SA_H;
			break;
		case OP_CRC32W :
			shamt_value = CRC32_SA_W;
			break;
		case OP_CRC32CB :
			shamt_value = CRC32_SA_C | CRC32_SA_B;
			break;
		case OP_CRC32CH :
			shamt_value = CRC32_SA_C | CRC32_SA_H;
			break;
		default :
			shamt_value = CRC32_SA_C | CRC32_SA_W;
			break;
	}
	instr->encoding = encode_r(OPCODE_SPECIAL3, instr->rs, instr->rt, 0, shamt_value, (uint32_t) funct_code);
}

static void build_target_instruction(Instruction *instr, int op_code)
{
	instr->encoding = ((uint32_t) op_code << OPCODE_SHIFT) | field(instr->target, 0, OFFSET_26_SIZE);
}

static void decode_fd_fs_operands(char *line, Instruction *instr)
{
	char fd[8], fs[8];
	if(sscanf(line, " $%7[^,] , $%7s ", fd, fs) != 2)
	{
		syntax_error(instr);
		return;
	}
	instr->fd = register_string_to_int(fd, REG_KIND_FPR);
	instr->fs = register_string_to_int(fs, REG_KIND_FPR);
	if(instr->fd == INVALID_REGISTER || instr->fs == INVALID_REGISTER) instr->error |= ERR_INVALID_REGISTER;
}

static void decode_fd_fs_ft_operands(char *line, Instruction *instr)
{
	char fd[8], fs[8], ft[8];
	if(sscanf(line, " $%7[^,] , $%7[^,] , $%7s ", fd, fs, ft) != 3)
	{
		syntax_error(instr);
		return;
	}
	instr->fd = register_string_to_int(fd, REG_KIND_FPR);
	instr->fs = register_string_to_int(fs, REG_KIND_FPR);
	instr->ft = register_string_to_int(ft, REG_KIND_FPR);
	if(instr->fd == INVALID_REGISTER || instr->fs == INVALID_REGISTER || instr->ft == INVALID_REGISTER) instr->error |= ERR_INVALID_REGISTER;
}

static void decode_ft_offset_operands(char *line, Instruction *instr)
{
	char ft[8], offset[16];
	if(sscanf(line, " $%7[^,] , %15s", ft, offset) != 2)
	{
		syntax_error(instr);
		return;
	}
	instr->ft = register_string_to_int(ft, REG_KIND_FPR);
	instr->offset = parse_integer(offset, instr);
	if(instr->ft == INVALID_REGISTER) instr->error |= ERR_INVALID_REGISTER;
}

static void decode_ft_offset_base_operands(char *line, Instruction *instr)
{
	char base[8], ft[8], offset[16];
	if(sscanf(line, " $%7[^,] , %15[^(] ($%7[^)]) ", ft, offset, base) != 3)
	{
		syntax_error(instr);
		return;
	}
	instr->base = register_string_to_int(base, REG_KIND_GPR);
	instr->offset = parse_integer(offset, instr);
	instr->ft = register_string_to_int(ft, REG_KIND_FPR);
	if(instr->base == INVALID_REGISTER || instr->ft == INVALID_REGISTER) instr->error |= ERR_INVALID_REGISTER;
}

static void decode_offset_operand(char *line, Instruction *instr)
{
	char offset[16];
	if(sscanf(line, " %15s ", offset) != 1)
	{
		syntax_error(instr);
		return;
	}
	instr->offset = parse_integer(offset, instr);
}

static void decode_rd_rs_operands(char *line, Instruction *instr)
{
	char rd[8], rs[8];
	/* Handling case of OP_JALR having two formats (quote from specification) :

	   Format:	OP_JALR rs (rd = 31 implied)
				OP_JALR rd, rs
	 */
	if(instr->opcode == OP_JALR)
	{
		if(strchr(line, ','))
		{
			if(sscanf(line, " $%7[^,] , $%7s ", rd, rs) != 2)
			{
				syntax_error(instr);
				return;
			}
			instr->rd = register_string_to_int(rd, REG_KIND_GPR);
		}
		else
		{
			if(sscanf(line, " $%7s ", rs) != 1)
			{
				syntax_error(instr);
				return;
			}
			instr->rd = REG_RA;
		}
	}
	else
	{
		if(sscanf(line, " $%7[^,] , $%7s ", rd, rs) != 2)
		{
			syntax_error(instr);
			return;
		}
		instr->rd = register_string_to_int(rd, REG_KIND_GPR);
	}
	instr->rs = register_string_to_int(rs, REG_KIND_GPR);
	if(instr->rd == INVALID_REGISTER || instr->rs == INVALID_REGISTER) instr->error |= ERR_INVALID_REGISTER;
}

static void decode_rd_rs_rt_operands(char *line, Instruction *instr)
{
	char rd[8], rs[8], rt[8];
	if(sscanf(line, " $%7[^,] , $%7[^,] , $%7s ", rd, rs, rt) != 3)
	{
		syntax_error(instr);
		return;
	}
	instr->rd = register_string_to_int(rd, REG_KIND_GPR);
	instr->rs = register_string_to_int(rs, REG_KIND_GPR);
	instr->rt = register_string_to_int(rt, REG_KIND_GPR);
	if(instr->rd == INVALID_REGISTER || instr->rs == INVALID_REGISTER || instr->rt == INVALID_REGISTER) instr->error |= ERR_INVALID_REGISTER;
}

/* rd, rs, rt, value : ALIGN (byte position) and LSA (scale), `value` goes to the sa field of the instruction */
static void decode_rd_rs_rt_value_operands(char *line, Instruction *instr, int32_t min, int32_t max, const char *name)
{
	char rd[8], rs[8], rt[8], value[16];
	if(sscanf(line, " $%7[^,] , $%7[^,] , $%7[^,] , %15s ", rd, rs, rt, value) != 4)
	{
		syntax_error(instr);
		return;
	}
	instr->rd = register_string_to_int(rd, REG_KIND_GPR);
	instr->rs = register_string_to_int(rs, REG_KIND_GPR);
	instr->rt = register_string_to_int(rt, REG_KIND_GPR);
	int32_t number = parse_integer(value, instr);
	if(number < min || number > max)
	{
		report("[!] %s out of range : %" PRId32 " (allowed : %" PRId32 " to %" PRId32 ")\n", name, number, min, max);
		instr->error |= ERR_RANGE;
		number = min;
	}
	instr->sa = (uint8_t) number;
	if(instr->rd == INVALID_REGISTER || instr->rs == INVALID_REGISTER || instr->rt == INVALID_REGISTER) instr->error |= ERR_INVALID_REGISTER;
}

static void decode_rd_rt_operands(char *line, Instruction *instr)
{
	char rd[8], rt[8];
	if(sscanf(line, " $%7[^,] , $%7s ", rd, rt) != 2)
	{
		syntax_error(instr);
		return;
	}
	instr->rd = register_string_to_int(rd, REG_KIND_GPR);
	instr->rt = register_string_to_int(rt, REG_KIND_GPR);
	if(instr->rd == INVALID_REGISTER || instr->rt == INVALID_REGISTER) instr->error |= ERR_INVALID_REGISTER;
}

static void decode_rd_rt_rs_operands(char *line, Instruction *instr)
{
	char rd[8], rs[8], rt[8];
	if(sscanf(line, " $%7[^,] , $%7[^,] , $%7s ", rd, rt, rs) != 3)
	{
		syntax_error(instr);
		return;
	}
	instr->rd = register_string_to_int(rd, REG_KIND_GPR);
	instr->rs = register_string_to_int(rs, REG_KIND_GPR);
	instr->rt = register_string_to_int(rt, REG_KIND_GPR);
	if(instr->rd == INVALID_REGISTER || instr->rs == INVALID_REGISTER || instr->rt == INVALID_REGISTER) instr->error |= ERR_INVALID_REGISTER;
}

static void decode_rd_rt_sa_operands(char *line, Instruction *instr)
{
	char rd[8], rt[8], sa[16];
	if(sscanf(line, " $%7[^,] , $%7[^,] , %15s ", rd, rt, sa) != 3)
	{
		syntax_error(instr);
		return;
	}
	instr->rd = register_string_to_int(rd, REG_KIND_GPR);
	instr->rt = register_string_to_int(rt, REG_KIND_GPR);
	int32_t sa_value = parse_integer(sa, instr);
	if(sa_value < 0 || sa_value > SA_MAX)
	{
		report("[!] Shift amount out of range : %" PRId32 " (allowed : 0 to %d)\n", sa_value, SA_MAX);
		instr->error |= ERR_RANGE;
		sa_value = 0;
	}
	instr->sa = (uint8_t) sa_value;
	if(instr->rd == INVALID_REGISTER || instr->rt == INVALID_REGISTER) instr->error |= ERR_INVALID_REGISTER;
}

static void decode_rs_operand(char *line, Instruction *instr)
{
	char rs[8];
	if(sscanf(line, " $%7s ", rs) != 1)
	{
		syntax_error(instr);
		return;
	}
	instr->rs = register_string_to_int(rs, REG_KIND_GPR);
	if(instr->rs == INVALID_REGISTER) instr->error |= ERR_INVALID_REGISTER;
}

static void decode_rs_immediate_operands(char *line, Instruction *instr)
{
	char imm[16], rs[8];
	if(sscanf(line, " $%7[^,], %15s ", rs, imm) != 2)
	{
		syntax_error(instr);
		return;
	}
	instr->immediate = parse_integer(imm, instr);
	instr->rs = register_string_to_int(rs, REG_KIND_GPR);
	if(instr->rs == INVALID_REGISTER) instr->error |= ERR_INVALID_REGISTER;
}

static void decode_rs_offset_operands(char *line, Instruction *instr)
{
	char offset[16], rs[8];
	if(sscanf(line, " $%7[^,]  , %15s ", rs, offset) != 2)
	{
		syntax_error(instr);
		return;
	}
	instr->offset = parse_integer(offset, instr);
	instr->rs = register_string_to_int(rs, REG_KIND_GPR);
	if(instr->rs == INVALID_REGISTER) instr->error |= ERR_INVALID_REGISTER;
}

static void decode_rs_rt_operands(char *line, Instruction *instr)
{
	char rs[8], rt[8];
	if(sscanf(line, " $%7[^,] , $%7s ", rs, rt) != 2)
	{
		syntax_error(instr);
		return;
	}
	instr->rs = register_string_to_int(rs, REG_KIND_GPR);
	instr->rt = register_string_to_int(rt, REG_KIND_GPR);
	if(instr->rs == INVALID_REGISTER || instr->rt == INVALID_REGISTER) instr->error |= ERR_INVALID_REGISTER;
}

static void decode_rs_rt_offset_operands(char *line, Instruction *instr)
{
	char offset[16], rs[8], rt[8];
	if(sscanf(line, " $%7[^,] , $%7[^,] , %15s ", rs, rt, offset) != 3)
	{
		syntax_error(instr);
		return;
	}
	instr->offset = parse_integer(offset, instr);
	instr->rs = register_string_to_int(rs, REG_KIND_GPR);
	instr->rt = register_string_to_int(rt, REG_KIND_GPR);
	if(instr->rs == INVALID_REGISTER || instr->rt == INVALID_REGISTER) instr->error |= ERR_INVALID_REGISTER;
}

static void decode_rt_fs_operands(char *line, Instruction *instr)
{
	char fs[8], rt[8];
	if(sscanf(line, " $%7[^,] , $%7s ", rt, fs) != 2)
	{
		syntax_error(instr);
		return;
	}
	instr->rt = register_string_to_int(rt, REG_KIND_GPR);
	instr->fs = register_string_to_int(fs, REG_KIND_FPR);
	if(instr->rt == INVALID_REGISTER || instr->fs == INVALID_REGISTER) instr->error |= ERR_INVALID_REGISTER;
}

static void decode_rt_immediate_operands(char *line, Instruction *instr)
{
	char imm[16], rt[8];
	if(sscanf(line, " $%7[^,] , %15s ", rt, imm) != 2)
	{
		syntax_error(instr);
		return;
	}
	instr->immediate = parse_integer(imm, instr);
	instr->rt = register_string_to_int(rt, REG_KIND_GPR);
	if(instr->rt == INVALID_REGISTER) instr->error |= ERR_INVALID_REGISTER;
}

static void decode_rt_offset_operands(char *line, Instruction *instr)
{
	char offset[16], rt[8];
	if(sscanf(line, " $%7[^,] , %15s ", rt, offset) != 2)
	{
		syntax_error(instr);
		return;
	}
	instr->offset = parse_integer(offset, instr);
	instr->rt = register_string_to_int(rt, REG_KIND_GPR);
	if(instr->rt == INVALID_REGISTER) instr->error |= ERR_INVALID_REGISTER;
}

static void decode_rt_offset_base_operands(char *line, Instruction *instr)
{
	char base[8], offset[16], rt[8];
	if(sscanf(line, " $%7[^,] , %15[^(] ($%7[^)]) ", rt, offset, base) != 3)
	{
		syntax_error(instr);
		return;
	}
	instr->base = register_string_to_int(base, REG_KIND_GPR);
	instr->offset = parse_integer(offset, instr);
	instr->rt = register_string_to_int(rt, REG_KIND_GPR);
	if(instr->base == INVALID_REGISTER || instr->rt == INVALID_REGISTER) instr->error |= ERR_INVALID_REGISTER;
}

static void decode_rt_rs_immediate_operands(char *line, Instruction *instr)
{
	char imm[16], rs[8], rt[8];
	if(sscanf(line, " $%7[^,] , $%7[^,] , %15s ", rt, rs, imm) != 3)
	{
		syntax_error(instr);
		return;
	}
	instr->immediate = parse_integer(imm, instr);
	instr->rs = register_string_to_int(rs, REG_KIND_GPR);
	instr->rt = register_string_to_int(rt, REG_KIND_GPR);
	if(instr->rs == INVALID_REGISTER || instr->rt == INVALID_REGISTER) instr->error |= ERR_INVALID_REGISTER;
}

/* CRC32x rt, rs, rt : the first and the last operands are the same register (rt is the CRC in and out) */
static void decode_rt_rs_rt_operands(char *line, Instruction *instr)
{
	char rs[8], rt[8], rt_again[8];
	if(sscanf(line, " $%7[^,] , $%7[^,] , $%7s ", rt, rs, rt_again) != 3)
	{
		syntax_error(instr);
		return;
	}
	instr->rt = register_string_to_int(rt, REG_KIND_GPR);
	instr->rs = register_string_to_int(rs, REG_KIND_GPR);
	if(instr->rt == INVALID_REGISTER || instr->rs == INVALID_REGISTER || register_string_to_int(rt_again, REG_KIND_GPR) == INVALID_REGISTER)
		instr->error |= ERR_INVALID_REGISTER;
	else if(register_string_to_int(rt_again, REG_KIND_GPR) != instr->rt) constraint_error(instr, "the first and the last operands must be the same register");
}

static void decode_rt_rs_offset_operands(char *line, Instruction *instr)
{
	char offset[16], rs[8], rt[8];
	if(sscanf(line, " $%7[^,] , $%7[^,] , %15s ", rt, rs, offset) != 3)
	{
		syntax_error(instr);
		return;
	}
	instr->offset = parse_integer(offset, instr);
	instr->rs = register_string_to_int(rs, REG_KIND_GPR);
	instr->rt = register_string_to_int(rt, REG_KIND_GPR);
	if(instr->rs == INVALID_REGISTER || instr->rt == INVALID_REGISTER) instr->error |= ERR_INVALID_REGISTER;
}

/* EXT / INS rt, rs, pos, size : the field must fit in a word (pos + size <= WORD_BITS) */
static void decode_rt_rs_pos_size_operands(char *line, Instruction *instr)
{
	char rs[8], rt[8], pos[16], size[16];
	if(sscanf(line, " $%7[^,] , $%7[^,] , %15[^,] , %15s ", rt, rs, pos, size) != 4)
	{
		syntax_error(instr);
		return;
	}
	instr->rt = register_string_to_int(rt, REG_KIND_GPR);
	instr->rs = register_string_to_int(rs, REG_KIND_GPR);
	if(instr->rt == INVALID_REGISTER || instr->rs == INVALID_REGISTER) instr->error |= ERR_INVALID_REGISTER;
	const int32_t pos_value = parse_integer(pos, instr);
	const int32_t size_value = parse_integer(size, instr);
	if(instr->error != NO_ERR) return;
	if(pos_value < 0 || pos_value > SA_MAX || size_value < 1 || size_value > WORD_BITS || pos_value + size_value > WORD_BITS)
	{
		report("[!] Bit field out of range : pos %" PRId32 ", size %" PRId32 " (allowed : pos 0 to %d, size 1 to %d, pos + size <= %d)\n", pos_value, size_value, SA_MAX, WORD_BITS, WORD_BITS);
		instr->error |= ERR_RANGE;
		return;
	}
	instr->pos = (uint8_t) pos_value;
	instr->size = (uint8_t) size_value;
}

static void decode_target_operands(char *line, Instruction *instr)
{
	char target[16];
	if(sscanf(line, " %15s ", target) != 1)
	{
		syntax_error(instr);
		return;
	}
	int32_t target_value = parse_integer(target, instr);
	if(target_value < 0 || target_value > TARGET_MAX)
	{
		report("[!] Target out of range : %" PRId32 " (allowed : 0 to %" PRId32 ")\n", target_value, TARGET_MAX);
		instr->error |= ERR_RANGE;
		target_value = 0;
	}
	instr->target = target_value;
}

/* Assembles the operands of an instruction (the text after the mnemonic) : fills `instr`, its encoding and its text */
void decode_assemble(const InstructionDesc *id, char *ptr, Instruction *instr)
{
	instr->opcode = id->op;
	switch(id->format)
	{
		case FMT_EXIT :
			instr->exit = 1;
			break;
		case FMT_NAL :
			build_cmd_instruction(instr, 0, OPCODE_REGIMM, 0, NAL_RT, 0, 0);
			snprintf(instr->to_string, sizeof(instr->to_string), "NAL");
			break;
		case FMT_NOP :
			build_cmd_instruction(instr, FUNCT_SLL, OPCODE_SPECIAL, 0, 0, 0, 0);
			snprintf(instr->to_string, sizeof(instr->to_string), "NOP");
			break;
		case FMT_FD_FS :
			decode_fd_fs_operands(ptr, instr);
			build_fd_fs_instruction(instr, id->code, id->fmt);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $f%d, $f%d", id->mnemonic, instr->fd, instr->fs);
			break;
		case FMT_FD_FS_FT :
			decode_fd_fs_ft_operands(ptr, instr);
			build_fd_fs_ft_instruction(instr, id->code, id->fmt);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $f%d, $f%d, $f%d", id->mnemonic, instr->fd, instr->fs, instr->ft);
			break;
		case FMT_FT_OFFSET_16 :
			decode_ft_offset_operands(ptr, instr);
			build_ft_offset_16_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $f%d, %d", id->mnemonic, instr->ft, instr->offset);
			break;
		case FMT_FT_OFFSET_16_BASE :
			decode_ft_offset_base_operands(ptr, instr);
			build_ft_offset_16_base_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $f%d, %d($%d)", id->mnemonic, instr->ft, instr->offset, instr->base);
			break;
		case FMT_RT_FS :
			decode_rt_fs_operands(ptr, instr);
			build_rt_fs_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $%d, $f%d", id->mnemonic, instr->rt, instr->fs);
			break;
		case FMT_OFFSET_16 :
			decode_offset_operand(ptr, instr);
			build_offset_16_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s %d", id->mnemonic, instr->offset);
			break;
		case FMT_OFFSET_26 :
			decode_offset_operand(ptr, instr);
			build_offset_26_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s %d", id->mnemonic, instr->offset);
			break;
		case FMT_RD_RS :
			decode_rd_rs_operands(ptr, instr);
			build_rd_rs_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $%d, $%d", id->mnemonic, instr->rd, instr->rs);
			break;
		case FMT_RD_RS_RT :
			decode_rd_rs_rt_operands(ptr, instr);
			build_rd_rs_rt_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $%d, $%d, $%d", id->mnemonic, instr->rd, instr->rs, instr->rt);
			break;
		case FMT_RD_RS_RT_BP :
			decode_rd_rs_rt_value_operands(ptr, instr, 0, ALIGN_BP_MAX, "Byte position");
			build_rd_rs_rt_bp_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $%d, $%d, $%d, %d", id->mnemonic, instr->rd, instr->rs, instr->rt, instr->sa);
			break;
		case FMT_RD_RS_RT_SA :
			decode_rd_rs_rt_value_operands(ptr, instr, 1, LSA_SA_MAX, "Scale");
			build_rd_rs_rt_sa_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $%d, $%d, $%d, %d", id->mnemonic, instr->rd, instr->rs, instr->rt, instr->sa);
			break;
		case FMT_RD_RT :
			decode_rd_rt_operands(ptr, instr);
			build_rd_rt_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $%d, $%d", id->mnemonic, instr->rd, instr->rt);
			break;
		case FMT_RD_RT_RS :
			decode_rd_rt_rs_operands(ptr, instr);
			build_rd_rt_rs_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $%d, $%d, $%d", id->mnemonic, instr->rd, instr->rt, instr->rs);
			break;
		case FMT_RD_RT_SA :
			decode_rd_rt_sa_operands(ptr, instr);
			build_rd_rt_sa_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $%d, $%d, %d", id->mnemonic, instr->rd, instr->rt, instr->sa);
			break;
		case FMT_RS :
			decode_rs_operand(ptr, instr);
			build_rs_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $%d", id->mnemonic, instr->rs);
			break;
		case FMT_RS_IMMEDIATE_16 :
			decode_rs_immediate_operands(ptr, instr);
			build_rs_immediate_16_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $%d, %d", id->mnemonic, instr->rs, instr->immediate);
			break;
		case FMT_RS_IMMEDIATE_19 :
			decode_rs_immediate_operands(ptr, instr);
			build_rs_immediate_19_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $%d, %d", id->mnemonic, instr->rs, instr->immediate);
			break;
		case FMT_RS_OFFSET_16 :
			decode_rs_offset_operands(ptr, instr);
			build_rs_offset_16_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $%d, %d", id->mnemonic, instr->rs, instr->offset);
			break;
		case FMT_RS_OFFSET_19 :
			decode_rs_offset_operands(ptr, instr);
			build_rs_offset_19_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $%d, %d", id->mnemonic, instr->rs, instr->offset);
			break;
		case FMT_RS_OFFSET_21 :
			decode_rs_offset_operands(ptr, instr);
			build_rs_offset_21_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $%d, %d", id->mnemonic, instr->rs, instr->offset);
			break;
		case FMT_RS_RT :
			decode_rs_rt_operands(ptr, instr);
			build_rs_rt_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $%d, $%d", id->mnemonic, instr->rs, instr->rt);
			break;
		case FMT_RS_RT_OFFSET_16 :
			decode_rs_rt_offset_operands(ptr, instr);
			build_rs_rt_offset_16_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $%d, $%d, %d", id->mnemonic, instr->rs, instr->rt, instr->offset);
			break;
		case FMT_RT_IMMEDIATE_16 :
			decode_rt_immediate_operands(ptr, instr);
			build_rt_immediate_16_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $%d, %d", id->mnemonic, instr->rt, instr->immediate);
			break;
		case FMT_RT_OFFSET_16 :
			decode_rt_offset_operands(ptr, instr);
			build_rt_offset_16_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $%d, %d", id->mnemonic, instr->rt, instr->offset);
			break;
		case FMT_RT_OFFSET_BASE :
			decode_rt_offset_base_operands(ptr, instr);
			build_rt_offset_16_base_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $%d, %d($%d)", id->mnemonic, instr->rt, instr->offset, instr->base);
			break;
		case FMT_RT_RS_IMMEDIATE_16 :
			decode_rt_rs_immediate_operands(ptr, instr);
			build_rt_rs_immediate_16_instruction(instr, id->code);
			/* Unsigned fields are shown as the manual defines them (ORI $9, $0, 65535, not -1) */
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $%d, $%d, %d", id->mnemonic, instr->rt, instr->rs,
			         decode_immediate_range(id->op) == RANGE_UNSIGNED ? (int32_t) ((uint32_t) instr->immediate & low_mask(IMM_16_SIZE)) : instr->immediate);
			break;
		case FMT_RT_RS_OFFSET_16 :
			decode_rt_rs_offset_operands(ptr, instr);
			build_rs_rt_offset_16_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $%d, $%d, %d", id->mnemonic, instr->rt, instr->rs, instr->offset);
			break;
		case FMT_RT_RS_POS_SIZE :
			decode_rt_rs_pos_size_operands(ptr, instr);
			build_rt_rs_pos_size_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $%d, $%d, %d, %d", id->mnemonic, instr->rt, instr->rs, instr->pos, instr->size);
			break;
		case FMT_RT_RS_RT :
			decode_rt_rs_rt_operands(ptr, instr);
			build_rt_rs_rt_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s $%d, $%d, $%d", id->mnemonic, instr->rt, instr->rs, instr->rt);
			break;
		case FMT_TARGET :
			decode_target_operands(ptr, instr);
			build_target_instruction(instr, id->code);
			snprintf(instr->to_string, sizeof(instr->to_string), "%s %d", id->mnemonic, instr->target);
			break;
	}
}

/* `hex` : the lines are 32-bit hexadecimal words, otherwise they are assembly (a file is one or the other) */
Instruction decode_instruction(int mode, FILE *input, int hex)
{
	char mnemonic[16], *line, *ptr;
	FILE *in = mode == INTERACTIVE ? stdin : input;
	Instruction instr = {0};
	int consumed = 0;

	line = read_full_line(in);
	if(!line)
	{
		instr.exit = 1;
		return instr;
	}
	line[strcspn(line, "\n")] = '\0';

	ptr = line;
	while(*ptr == ' ' || *ptr == '\t') ptr++;
	if(*ptr == '#' || *ptr == '\0')
	{
		free(line);
		return(instr);
	}

	if(hex)
	{
		hex_decode_line(ptr, &instr);
		if(instr.error != NO_ERR) instr.opcode = OP_MIN;
		else log_instruction(&instr);
		free(line);
		return(instr);
	}

	sscanf(ptr, "%15s%n", mnemonic, &consumed);
	ptr += consumed;

	const InstructionDesc *id = find_instruction(mnemonic);

	if(id)
	{
		// if(chunk[0] == '.') printf("[!] Directives are currently not supported.\n"); TODO
		decode_assemble(id, ptr, &instr);
		if(instr.error != NO_ERR)
		{
			fprintf(stderr, "[!] Instruction rejected : %s\n", line);
			instr.opcode = OP_MIN;
		}
		else log_instruction(&instr);
	}
	else
	{
		fprintf(stderr, "[!] Unknown instruction : %s%s\n", mnemonic, hex_is_word(mnemonic) ? " (a hexadecimal word, use the -hex option)" : "");
		instr.error |= ERR_UNKNOWN_INSTRUCTION;
	}

	free(line);

	return(instr);
}
