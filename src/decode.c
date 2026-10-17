#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decode.h"
#include "display.h"
#include "instruction.h"
#include "mips_encoding.h"
#include "mips_registers.h"
#include "utils.h"

#define SA_MAX     31
#define TARGET_MAX ((INT32_C(1) << OFFSET_26_SIZE) - 1)

static void syntax_error(Instruction *instr)
{
	fprintf(stderr, "[!] Syntax error : missing or malformed operands\n");
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
		fprintf(stderr, "[!] Invalid number : '%s'\n", str);
		instr->error |= ERR_INVALID_NUMBER;
		return(0);
	}

	while(isspace((unsigned char) *end)) end++;

	if(*end != '\0')
	{
		fprintf(stderr, "[!] Invalid number : '%s'\n", str);
		instr->error |= ERR_INVALID_NUMBER;
		return(0);
	}

	if(errno == ERANGE || value > INT32_MAX || value < INT32_MIN)
	{
		fprintf(stderr, "[!] Number out of range : '%s'\n", str);
		instr->error |= ERR_RANGE;
		return(0);
	}

	return((int32_t) value);
}

/* A value is accepted if it fits in `size` bits either as signed or as unsigned,
   i.e. [-2^(size-1); 2^size - 1]. Returns it sign-extended, as the execution expects. */
static int32_t check_field(Instruction *instr, int32_t value, uint8_t size)
{
	int32_t min = -(INT32_C(1) << (size - 1)), max = (INT32_C(1) << size) - 1;

	if(value < min || value > max)
	{
		fprintf(stderr, "[!] Value out of range : %" PRId32 " does not fit in %d bits (allowed : %" PRId32 " to %" PRId32 ")\n", value, size, min, max);
		instr->error |= ERR_RANGE;
		return(0);
	}

	return(sign_extend(value, size));
}

static uint32_t field(int32_t value, uint8_t shift, uint8_t size)
{
	return(((uint32_t) value & ((UINT32_C(1) << size) - 1)) << shift);
}

static uint32_t encode_r(uint32_t op, uint32_t rs, uint32_t rt, uint32_t rd, uint32_t shamt, uint32_t funct)
{
	return((op << OPCODE_SHIFT) | (rs << RS_SHIFT) | (rt << RT_SHIFT) | (rd << RD_SHIFT) | (shamt << SHAMT_SHIFT) | (funct << FUNCT_SHIFT));
}

/* `value` is the already checked and sign-extended immediate/offset */
static uint32_t encode_i(uint32_t op, uint32_t rs, uint32_t rt, int32_t value, uint8_t size)
{
	return((op << OPCODE_SHIFT) | (rs << RS_SHIFT) | (rt << RT_SHIFT) | field(value, 0, size));
}

static void build_cmd_instruction(Instruction *instr, int funct_code, int op_code, int rs_value, int rt_value, int rd_value, int shamt_value)
{
	instr->encoding = encode_r((uint32_t) op_code, (uint32_t) rs_value, (uint32_t) rt_value, (uint32_t) rd_value, (uint32_t) shamt_value, (uint32_t) funct_code);
}

static void build_offset_16_instruction(Instruction *instr, int op_code)
{
	uint32_t rt_value = 0;
	if(instr->opcode == OP_BAL) rt_value = BAL_RT;
	instr->offset = check_field(instr, instr->offset, OFFSET_16_SIZE);
	instr->encoding = encode_i((uint32_t) op_code, 0, rt_value, instr->offset, OFFSET_16_SIZE);
}

static void build_offset_26_instruction(Instruction *instr, int op_code)
{
	instr->offset = check_field(instr, instr->offset, OFFSET_26_SIZE);
	instr->encoding = ((uint32_t) op_code << OPCODE_SHIFT) | field(instr->offset, 0, OFFSET_26_SIZE);
}

static void build_rd_rs_instruction(Instruction *instr, int funct_code)
{
	uint32_t shamt_value = 0;
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

static void build_rd_rt_instruction(Instruction *instr, int funct_code)
{
	uint32_t shamt_value = 0;
	if(instr->opcode == OP_SEB) shamt_value = SEB_SA;
	else if(instr->opcode == OP_SEH) shamt_value = SEH_SA;
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
	instr->immediate = check_field(instr, instr->immediate, IMM_16_SIZE);
	instr->encoding = encode_i((uint32_t) op_code, instr->rs, rt_value, instr->immediate, IMM_16_SIZE);
}

static void build_rs_immediate_19_instruction(Instruction *instr, int op_code)
{
	instr->immediate = check_field(instr, instr->immediate, IMM_19_SIZE);
	instr->encoding = encode_i((uint32_t) op_code, instr->rs, 0, instr->immediate, IMM_19_SIZE);
}

static void build_rs_offset_16_instruction(Instruction *instr, int op_code)
{
	uint32_t rt_value = 0;
	if(instr->opcode == OP_BGEZ) rt_value = BGEZ_RT;
	instr->offset = check_field(instr, instr->offset, OFFSET_16_SIZE);
	instr->encoding = encode_i((uint32_t) op_code, instr->rs, rt_value, instr->offset, OFFSET_16_SIZE);
}

static void build_rs_offset_19_instruction(Instruction *instr, int op_code)
{
	uint32_t rt_value = 0;
	if(instr->opcode == OP_LWPC) rt_value = LWPC_RT;
	instr->offset = check_field(instr, instr->offset, OFFSET_19_SIZE);
	instr->encoding = encode_i((uint32_t) op_code, instr->rs, rt_value, instr->offset, OFFSET_19_SIZE);
}

static void build_rs_offset_21_instruction(Instruction *instr, int op_code)
{
	instr->offset = check_field(instr, instr->offset, OFFSET_21_SIZE);
	instr->encoding = encode_i((uint32_t) op_code, instr->rs, 0, instr->offset, OFFSET_21_SIZE);
}

static void build_rs_rt_instruction(Instruction *instr, int funct_code)
{
	instr->encoding = encode_r(OPCODE_SPECIAL, instr->rs, instr->rt, 0, 0, (uint32_t) funct_code);
}

static void build_rs_rt_offset_16_instruction(Instruction *instr, int op_code)
{
	instr->offset = check_field(instr, instr->offset, OFFSET_16_SIZE);
	instr->encoding = encode_i((uint32_t) op_code, instr->rs, instr->rt, instr->offset, OFFSET_16_SIZE);
}

static void build_rt_immediate_16_instruction(Instruction *instr, int op_code)
{
	instr->immediate = check_field(instr, instr->immediate, IMM_16_SIZE);
	instr->encoding = encode_i((uint32_t) op_code, 0, instr->rt, instr->immediate, IMM_16_SIZE);
}

static void build_rt_offset_16_instruction(Instruction *instr, int op_code)
{
	instr->offset = check_field(instr, instr->offset, OFFSET_16_SIZE);
	instr->encoding = encode_i((uint32_t) op_code, 0, instr->rt, instr->offset, OFFSET_16_SIZE);
}

/* COP1 `fmt` field : single (S) or double (D) precision, deduced from the instruction */
static uint32_t cop1_format(Opcode opcode)
{
	switch(opcode)
	{
		case OP_ABS_D :
		case OP_ADD_D :
		case OP_DIV_D :
		case OP_MOV_D :
		case OP_MUL_D :
		case OP_NEG_D :
		case OP_SUB_D :
			return(COP1_FMT_D);
		default :
			return(COP1_FMT_S);
	}
}

static void build_fd_fs_instruction(Instruction *instr, int funct_code)
{
	instr->encoding = encode_r(OPCODE_COP1, cop1_format(instr->opcode), 0, instr->fs, instr->fd, (uint32_t) funct_code);
}

static void build_fd_fs_ft_instruction(Instruction *instr, int funct_code)
{
	instr->encoding = encode_r(OPCODE_COP1, cop1_format(instr->opcode), instr->ft, instr->fs, instr->fd, (uint32_t) funct_code);
}

static void build_ft_offset_base_instruction(Instruction *instr, int op_code)
{
	instr->offset = check_field(instr, instr->offset, OFFSET_16_SIZE);
	instr->encoding = encode_i((uint32_t) op_code, instr->base, instr->ft, instr->offset, OFFSET_16_SIZE);
}

static void build_rt_fs_instruction(Instruction *instr, int sub_code)
{
	instr->encoding = encode_r(OPCODE_COP1, (uint32_t) sub_code, instr->rt, instr->fs, 0, 0);
}

static void build_rt_offset_base_instruction(Instruction *instr, int op_code)
{
	instr->offset = check_field(instr, instr->offset, OFFSET_16_SIZE);
	instr->encoding = encode_i((uint32_t) op_code, instr->base, instr->rt, instr->offset, OFFSET_16_SIZE);
}

static void build_rt_rs_immediate_16_instruction(Instruction *instr, int op_code)
{
	instr->immediate = check_field(instr, instr->immediate, IMM_16_SIZE);
	instr->encoding = encode_i((uint32_t) op_code, instr->rs, instr->rt, instr->immediate, IMM_16_SIZE);
}

static void build_target_instruction(Instruction *instr, int op_code)
{
	instr->encoding = ((uint32_t) op_code << OPCODE_SHIFT) | field(instr->target, 0, OFFSET_26_SIZE);
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
		fprintf(stderr, "[!] Shift amount out of range : %" PRId32 " (allowed : 0 to %d)\n", sa_value, SA_MAX);
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
		fprintf(stderr, "[!] Target out of range : %" PRId32 " (allowed : 0 to %" PRId32 ")\n", target_value, TARGET_MAX);
		instr->error |= ERR_RANGE;
		target_value = 0;
	}
	instr->target = target_value;
}

Instruction decode_instruction(int mode, FILE *input)
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

	sscanf(ptr, "%15s%n", mnemonic, &consumed);
	ptr += consumed;

	const InstructionDesc *id = find_instruction(mnemonic);

	if(id)
	{
		// if(chunk[0] == '.') printf("[!] Directives are currently not supported.\n"); TODO
		instr.opcode = id->op;
		switch(id->format)
		{
			case FMT_EXIT :
				instr.exit = 1;
				break;
			case FMT_NAL :
				build_cmd_instruction(&instr, 0, OPCODE_REGIMM, 0, NAL_RT, 0, 0);
				snprintf(instr.to_string, sizeof(instr.to_string), "NAL -> 0x%08" PRIx32 "\n", instr.encoding);
				break;
			case FMT_NOP :
				build_cmd_instruction(&instr, FUNCT_SLL, OPCODE_SPECIAL, 0, 0, 0, 0);
				snprintf(instr.to_string, sizeof(instr.to_string), "NOP -> 0x%08" PRIx32 "\n", instr.encoding);
				break;
			case FMT_FD_FS :
				decode_fd_fs_operands(ptr, &instr);
				build_fd_fs_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $f%d, $f%d -> 0x%08" PRIx32 "\n", id->mnemonic, instr.fd, instr.fs, instr.encoding);
				break;
			case FMT_FD_FS_FT :
				decode_fd_fs_ft_operands(ptr, &instr);
				build_fd_fs_ft_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $f%d, $f%d, $f%d -> 0x%08" PRIx32 "\n", id->mnemonic, instr.fd, instr.fs, instr.ft, instr.encoding);
				break;
			case FMT_FT_OFFSET_BASE :
				decode_ft_offset_base_operands(ptr, &instr);
				build_ft_offset_base_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $f%d, %d($%d) -> 0x%08" PRIx32 "\n", id->mnemonic, instr.ft, instr.offset, instr.base, instr.encoding);
				break;
			case FMT_RT_FS :
				decode_rt_fs_operands(ptr, &instr);
				build_rt_fs_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $f%d -> 0x%08" PRIx32 "\n", id->mnemonic, instr.rt, instr.fs, instr.encoding);
				break;
			case FMT_OFFSET_16 :
				decode_offset_operand(ptr, &instr);
				build_offset_16_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s %d -> 0x%08" PRIx32 "\n", id->mnemonic, instr.offset, instr.encoding);
				break;
			case FMT_OFFSET_26 :
				decode_offset_operand(ptr, &instr);
				build_offset_26_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s %d -> 0x%08" PRIx32 "\n", id->mnemonic, instr.offset, instr.encoding);
				break;
			case FMT_RD_RS :
				decode_rd_rs_operands(ptr, &instr);
				build_rd_rs_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $%d -> 0x%08" PRIx32 "\n", id->mnemonic, instr.rd, instr.rs, instr.encoding);
				break;
			case FMT_RD_RS_RT :
				decode_rd_rs_rt_operands(ptr, &instr);
				build_rd_rs_rt_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $%d, $%d -> 0x%08" PRIx32 "\n", id->mnemonic, instr.rd, instr.rs, instr.rt, instr.encoding);
				break;
			case FMT_RD_RT :
				decode_rd_rt_operands(ptr, &instr);
				build_rd_rt_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $%d -> 0x%08" PRIx32 "\n", id->mnemonic, instr.rd, instr.rt, instr.encoding);
				break;
			case FMT_RD_RT_RS :
				decode_rd_rt_rs_operands(ptr, &instr);
				build_rd_rt_rs_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $%d, $%d -> 0x%08" PRIx32 "\n", id->mnemonic, instr.rd, instr.rt, instr.rs, instr.encoding);
				break;
			case FMT_RD_RT_SA :
				decode_rd_rt_sa_operands(ptr, &instr);
				build_rd_rt_sa_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $%d, %d -> 0x%08" PRIx32 "\n", id->mnemonic, instr.rd, instr.rt, instr.sa, instr.encoding);
				break;
			case FMT_RS :
				decode_rs_operand(ptr, &instr);
				build_rs_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d -> 0x%08" PRIx32 "\n", id->mnemonic, instr.rs, instr.encoding);
				break;
			case FMT_RS_IMMEDIATE_16 :
				decode_rs_immediate_operands(ptr, &instr);
				build_rs_immediate_16_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, %d -> 0x%08" PRIx32 "\n", id->mnemonic, instr.rs, instr.immediate, instr.encoding);
				break;
			case FMT_RS_IMMEDIATE_19 :
				decode_rs_immediate_operands(ptr, &instr);
				build_rs_immediate_19_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, %d -> 0x%08" PRIx32 "\n", id->mnemonic, instr.rs, instr.immediate, instr.encoding);
				break;
			case FMT_RS_OFFSET_16 :
				decode_rs_offset_operands(ptr, &instr);
				build_rs_offset_16_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, %d -> 0x%08" PRIx32 "\n", id->mnemonic, instr.rs, instr.offset, instr.encoding);
				break;
			case FMT_RS_OFFSET_19 :
				decode_rs_offset_operands(ptr, &instr);
				build_rs_offset_19_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, %d -> 0x%08" PRIx32 "\n", id->mnemonic, instr.rs, instr.offset, instr.encoding);
				break;
			case FMT_RS_OFFSET_21 :
				decode_rs_offset_operands(ptr, &instr);
				build_rs_offset_21_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, %d -> 0x%08" PRIx32 "\n", id->mnemonic, instr.rs, instr.offset, instr.encoding);
				break;
			case FMT_RS_RT :
				decode_rs_rt_operands(ptr, &instr);
				build_rs_rt_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $%d -> 0x%08" PRIx32 "\n", id->mnemonic, instr.rs, instr.rt, instr.encoding);
				break;
			case FMT_RS_RT_OFFSET_16 :
				decode_rs_rt_offset_operands(ptr, &instr);
				build_rs_rt_offset_16_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $%d, %d -> 0x%08" PRIx32 "\n", id->mnemonic, instr.rs, instr.rt, instr.offset, instr.encoding);
				break;
			case FMT_RT_IMMEDIATE_16 :
				decode_rt_immediate_operands(ptr, &instr);
				build_rt_immediate_16_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, %d -> 0x%08" PRIx32 "\n", id->mnemonic, instr.rt, instr.immediate, instr.encoding);
				break;
			case FMT_RT_OFFSET_16 :
				decode_rt_offset_operands(ptr, &instr);
				build_rt_offset_16_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s %d, %d -> 0x%08" PRIx32 "\n", id->mnemonic, instr.rt, instr.offset, instr.encoding);
				break;
			case FMT_RT_OFFSET_BASE :
				decode_rt_offset_base_operands(ptr, &instr);
				build_rt_offset_base_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, %d($%d) -> 0x%08" PRIx32 "\n", id->mnemonic, instr.rt, instr.offset, instr.base, instr.encoding);
				break;
			case FMT_RT_RS_IMMEDIATE_16 :
				decode_rt_rs_immediate_operands(ptr, &instr);
				build_rt_rs_immediate_16_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $%d, %d -> 0x%08" PRIx32 "\n", id->mnemonic, instr.rt, instr.rs, instr.immediate, instr.encoding);
				break;
			case FMT_RT_RS_OFFSET_16 :
				decode_rt_rs_offset_operands(ptr, &instr);
				build_rs_rt_offset_16_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s $%d, $%d, %d -> 0x%08" PRIx32 "\n", id->mnemonic, instr.rt, instr.rs, instr.offset, instr.encoding);
				break;
			case FMT_TARGET :
				decode_target_operands(ptr, &instr);
				build_target_instruction(&instr, id->code);
				snprintf(instr.to_string, sizeof(instr.to_string), "%s %d -> 0x%08" PRIx32 "\n", id->mnemonic, instr.target, instr.encoding);
				break;
		}
		if(instr.error != NO_ERR)
		{
			fprintf(stderr, "[!] Instruction rejected : %s\n", line);
			instr.opcode = OP_MIN;
		}
		else log_instruction(&instr);
	}
	else
	{
		fprintf(stderr, "[!] Unknown instruction : %s\n", mnemonic);
		instr.error |= ERR_UNKNOWN_INSTRUCTION;
	}

	free(line);

	return(instr);
}
