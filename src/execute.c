#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "cpu.h"
#include "execute.h"
#include "mips_encoding.h"
#include "mips_memory.h"
#include "mips_registers.h"
#include "utils.h"

/* An invalid operation (inf - inf, 0 / 0...) produces the MIPS default quiet NaN, whereas the host
   produces its own NaN pattern. A NaN operand is propagated as it is, so the host result is kept. */
static float fp_result_float(float result, float a, float b)
{
	const uint32_t default_nan = 0x7FC00000u;
	float nan;

	if(!isnan(result) || isnan(a) || isnan(b)) return(result);

	memcpy(&nan, &default_nan, sizeof(nan));

	return(nan);
}

static double fp_result_double(double result, double a, double b)
{
	const uint64_t default_nan = 0x7FF8000000000000u;
	double nan;

	if(!isnan(result) || isnan(a) || isnan(b)) return(result);

	memcpy(&nan, &default_nan, sizeof(nan));

	return(nan);
}

/* PC-relative branch target : the offset is counted in instructions, from the one following the branch */
static uint32_t branch_target(const Cpu *cpu, int32_t offset)
{
	return(cpu->PC + ((uint32_t) offset << 2) + 4);
}

/* Branch with a delay slot : the jump is applied after the next instruction has been executed */
static void branch_delay_slot(Cpu *cpu, int32_t offset)
{
	cpu->next_PC = branch_target(cpu, offset);
}

/* MIPS32 Release 6 compact branch : no delay slot, the PC is updated immediately.
   Returns 1 so that the caller skips the final PC += 4 */
static uint8_t branch_compact(Cpu *cpu, int32_t offset)
{
	cpu->PC = branch_target(cpu, offset);
	return(1);
}

/* Same as branch_compact(), and stores the return address in $ra */
static uint8_t branch_compact_link(Cpu *cpu, int32_t offset)
{
	gpr_write_32(cpu, REG_RA, cpu->PC + 4);
	return(branch_compact(cpu, offset));
}

void execute_instruction(Cpu *cpu, Config *cfg, Instruction *instr)
{
	double double_a, double_b;
	float single_a, single_b;
	int32_t dividend, divisor, i32;
	int64_t i64;
	uint16_t u16;
	uint32_t address, pending_PC, raw_dividend, raw_divisor, u32;
	uint64_t u64;
	uint8_t compact = 0, u8;

	pending_PC = cpu->next_PC;
	cpu->next_PC = INVALID_PC;

	switch(instr->opcode)
	{
		case OP_ABS_D :
			u64 = fpr_read_64(cpu, instr->fs) & 0x7FFFFFFFFFFFFFFFu;
			fpr_write_64(cpu, instr->fd, u64);
			break;
		case OP_ABS_S :
			u32 = fpr_read_32(cpu, instr->fs) & 0x7FFFFFFFu;
			fpr_write_32(cpu, instr->fd, u32);
			break;
		case OP_ADD :
			i64 = (int64_t) (int32_t) gpr_read_32(cpu, instr->rs) + (int64_t) (int32_t) gpr_read_32(cpu, instr->rt);
			if(i64 > INT32_MAX || i64 < INT32_MIN) fprintf(stderr, "[!] Exception : Integer Overflow\n");
			else gpr_write_32(cpu, instr->rd, (uint32_t) i64);
			break;
		case OP_ADDIU :
			u32 = gpr_read_32(cpu, instr->rs) + (uint32_t) instr->immediate;
			gpr_write_32(cpu, instr->rt, u32);
			break;
		case OP_ADDIUPC :
			u32 = cpu->PC + ((uint32_t) instr->immediate << 2);
			gpr_write_32(cpu, instr->rs, u32);
			break;
		case OP_ADDU :
			u32 = gpr_read_32(cpu, instr->rs) + gpr_read_32(cpu, instr->rt);
			gpr_write_32(cpu, instr->rd, u32);
			break;
		case OP_ADD_D :
			double_a = fpr_read_double(cpu, instr->fs);
			double_b = fpr_read_double(cpu, instr->ft);
			fpr_write_double(cpu, instr->fd, fp_result_double(double_a + double_b, double_a, double_b));
			break;
		case OP_ADD_S :
			single_a = fpr_read_float(cpu, instr->fs);
			single_b = fpr_read_float(cpu, instr->ft);
			fpr_write_float(cpu, instr->fd, fp_result_float(single_a + single_b, single_a, single_b));
			break;
		case OP_ALUIPC :
			u32 = ~0xFFFFu & (cpu->PC + ((uint32_t) instr->immediate << 16));
			gpr_write_32(cpu, instr->rs, u32);
			break;
		case OP_AND :
			u32 = gpr_read_32(cpu, instr->rs) & gpr_read_32(cpu, instr->rt);
			gpr_write_32(cpu, instr->rd, u32);
			break;
		case OP_ANDI :
			u32 = gpr_read_32(cpu, instr->rs) & ((uint32_t) instr->immediate & 0xFFFF);
			gpr_write_32(cpu, instr->rt, u32);
			break;
		case OP_AUI :
			u32 = gpr_read_32(cpu, instr->rs) + ((uint32_t) instr->immediate << 16);
			gpr_write_32(cpu, instr->rt, u32);
			break;
		case OP_AUIPC :
			u32 = cpu->PC + ((uint32_t) instr->immediate << 16);
			gpr_write_32(cpu, instr->rs, u32);
			break;
		case OP_BAL :
			gpr_write_32(cpu, REG_RA, cpu->PC + 8);
			branch_delay_slot(cpu, instr->offset);
			break;
		case OP_BALC :
			compact = branch_compact_link(cpu, instr->offset);
			break;
		case OP_BC :
			compact = branch_compact(cpu, instr->offset);
			break;
		case OP_BC1EQZ :
			u64 = fpr_read_64(cpu, instr->ft);
			if(!(u64 & 0x1)) branch_delay_slot(cpu, instr->offset);
			break;
		case OP_BC1NEZ :
			u64 = fpr_read_64(cpu, instr->ft);
			if(u64 & 0x1) branch_delay_slot(cpu, instr->offset);
			break;
		case OP_BEQ :
			if(gpr_read_32(cpu, instr->rs) == gpr_read_32(cpu, instr->rt)) branch_delay_slot(cpu, instr->offset);
			break;
		case OP_BEQC :
			if(gpr_read_32(cpu, instr->rs) == gpr_read_32(cpu, instr->rt)) compact = branch_compact(cpu, instr->offset);
			break;
		case OP_BEQZALC :
			if(gpr_read_32(cpu, instr->rt) == 0) compact = branch_compact_link(cpu, instr->offset);
			break;
		case OP_BEQZC :
			if(gpr_read_32(cpu, instr->rs) == 0) compact = branch_compact(cpu, instr->offset);
			break;
		case OP_BGEC :
			if((int32_t) gpr_read_32(cpu, instr->rs) >= (int32_t) gpr_read_32(cpu, instr->rt)) compact = branch_compact(cpu, instr->offset);
			break;
		case OP_BGEUC :
			if(gpr_read_32(cpu, instr->rs) >= gpr_read_32(cpu, instr->rt)) compact = branch_compact(cpu, instr->offset);
			break;
		case OP_BGEZ :
			if((int32_t) gpr_read_32(cpu, instr->rs) >= 0)
				branch_delay_slot(cpu, instr->offset);
			break;
		case OP_BGEZALC :
			if((int32_t) gpr_read_32(cpu, instr->rt) >= 0) compact = branch_compact_link(cpu, instr->offset);
			break;
		case OP_BGEZC :
			if((int32_t) gpr_read_32(cpu, instr->rt) >= 0) compact = branch_compact(cpu, instr->offset);
			break;
		case OP_BGTZ :
			if((int32_t) gpr_read_32(cpu, instr->rs) > 0)
				branch_delay_slot(cpu, instr->offset);
			break;
		case OP_BGTZALC :
			if((int32_t) gpr_read_32(cpu, instr->rt) > 0) compact = branch_compact_link(cpu, instr->offset);
			break;
		case OP_BGTZC :
			if((int32_t) gpr_read_32(cpu, instr->rt) > 0) compact = branch_compact(cpu, instr->offset);
			break;
		case OP_BITSWAP :
			/* Optimized algorithm I thought was interesting */
			u32 = gpr_read_32(cpu, instr->rt);
			u32 = ((u32 >> 1) & 0x55555555) | ((u32 & 0x55555555) << 1);
			u32 = ((u32 >> 2) & 0x33333333) | ((u32 & 0x33333333) << 2);
			u32 = ((u32 >> 4) & 0x0F0F0F0F) | ((u32 & 0x0F0F0F0F) << 4);
			gpr_write_32(cpu, instr->rd, u32);
			break;
		case OP_BLEZ :
			if((int32_t) gpr_read_32(cpu, instr->rs) <= 0)
				branch_delay_slot(cpu, instr->offset);
			break;
		case OP_BLEZALC :
			if((int32_t) gpr_read_32(cpu, instr->rt) <= 0) compact = branch_compact_link(cpu, instr->offset);
			break;
		case OP_BLEZC :
			if((int32_t) gpr_read_32(cpu, instr->rt) <= 0) compact = branch_compact(cpu, instr->offset);
			break;
		case OP_BLTC :
			if((int32_t) gpr_read_32(cpu, instr->rs) < (int32_t) gpr_read_32(cpu, instr->rt)) compact = branch_compact(cpu, instr->offset);
			break;
		case OP_BLTUC :
			if(gpr_read_32(cpu, instr->rs) < gpr_read_32(cpu, instr->rt)) compact = branch_compact(cpu, instr->offset);
			break;
		case OP_BLTZ :
			if((int32_t) gpr_read_32(cpu, instr->rs) < 0)
				branch_delay_slot(cpu, instr->offset);
			break;
		case OP_BLTZALC :
			if((int32_t) gpr_read_32(cpu, instr->rt) < 0) compact = branch_compact_link(cpu, instr->offset);
			break;
		case OP_BLTZC :
			if((int32_t) gpr_read_32(cpu, instr->rt) < 0) compact = branch_compact(cpu, instr->offset);
			break;
		case OP_BNE :
			if(gpr_read_32(cpu, instr->rs) != gpr_read_32(cpu, instr->rt))
				branch_delay_slot(cpu, instr->offset);
			break;
		case OP_BNEC :
			if(gpr_read_32(cpu, instr->rs) != gpr_read_32(cpu, instr->rt)) compact = branch_compact(cpu, instr->offset);
			break;
		case OP_BNEZALC :
			if((int32_t) gpr_read_32(cpu, instr->rt) != 0) compact = branch_compact_link(cpu, instr->offset);
			break;
		case OP_BNEZC :
			if((int32_t) gpr_read_32(cpu, instr->rs) != 0) compact = branch_compact(cpu, instr->offset);
			break;
		case OP_BNVC :
			i64 = (int64_t) (int32_t) gpr_read_32(cpu, instr->rs) + (int64_t) (int32_t) gpr_read_32(cpu, instr->rt);
			if(i64 <= INT32_MAX && i64 >= INT32_MIN) compact = branch_compact(cpu, instr->offset);
			break;
		case OP_BOVC :
			i64 = (int64_t) (int32_t) gpr_read_32(cpu, instr->rs) + (int64_t) (int32_t) gpr_read_32(cpu, instr->rt);
			if(i64 > INT32_MAX || i64 < INT32_MIN) compact = branch_compact(cpu, instr->offset);
			break;
		case OP_CLO :
			/* Naive OP_CLO/Z implementation, didn't want to just
			use C __builtin_clz/popu8 */
			u32 = gpr_read_32(cpu, instr->rs);
			if(u32 == 0xFFFFFFFFu) gpr_write_32(cpu, instr->rd, 32);
			else
			{
				u8 = 0;
				while(u32 & 0x80000000)
				{
					u32 <<= 1;
					u8++;
				}
				gpr_write_32(cpu, instr->rd, (uint32_t) u8);
			}
			break;
		case OP_CLZ :
			u32 = gpr_read_32(cpu, instr->rs);
			if(u32 == 0u) gpr_write_32(cpu, instr->rd, 32);
			else
			{
				u8 = 0;
				while(!(u32 & 0x80000000))
				{
					u32 <<= 1;
					u8++;
				}
				gpr_write_32(cpu, instr->rd, (uint32_t) u8);
			}
			break;
		case OP_DIV :
			dividend = (int32_t) gpr_read_32(cpu, instr->rs);
			divisor = (int32_t) gpr_read_32(cpu, instr->rt);
			/* Arbitrary choice, MIPS32 documentation :
			'If the divisor in GPR rt is zero, the arithmetic result value is UNPREDICTABLE' */
			if(divisor == 0) i32 = divisor;
			/* Used to cover a C edge-case of integer overflow - not MIPS related */
			else if(divisor == -1 && dividend == (int32_t) 0x80000000) i32 = dividend;
			else i32 = dividend / divisor;
			gpr_write_32(cpu, instr->rd, (uint32_t) i32);
			break;
		case OP_DIVU :
			raw_dividend = gpr_read_32(cpu, instr->rs);
			raw_divisor = gpr_read_32(cpu, instr->rt);
			/* Arbitrary choice, MIPS32 documentation :
			'If the divisor in GPR rt is zero, the arithmetic result value is UNPREDICTABLE' */
			if(raw_divisor == 0) u32 = raw_divisor;
			/* No integer overflow with unsigned instruction */
			else u32 = raw_dividend / raw_divisor;
			gpr_write_32(cpu, instr->rd, u32);
			break;
		case OP_DIV_D :
			double_a = fpr_read_double(cpu, instr->fs);
			double_b = fpr_read_double(cpu, instr->ft);
			fpr_write_double(cpu, instr->fd, fp_result_double(double_a / double_b, double_a, double_b));
			break;
		case OP_DIV_S :
			single_a = fpr_read_float(cpu, instr->fs);
			single_b = fpr_read_float(cpu, instr->ft);
			fpr_write_float(cpu, instr->fd, fp_result_float(single_a / single_b, single_a, single_b));
			break;
		case OP_J :
			cpu->next_PC = ((cpu->PC + 4) & 0xF0000000u) | ((uint32_t) (instr->target << 2) & 0x0FFFFFFCu);
			break;
		case OP_JAL :
			gpr_write_32(cpu, REG_RA, cpu->PC + 8);
			cpu->next_PC = ((cpu->PC + 4) & 0xF0000000u) | ((uint32_t) (instr->target << 2) & 0x0FFFFFFCu);
			break;
		case OP_JALR :
			gpr_write_32(cpu, instr->rd, cpu->PC + 8);
			if(gpr_read_32(cpu, instr->rs) & 0x03u) fprintf(stderr, "[!] Exception : Address Error\n");
			else cpu->next_PC = gpr_read_32(cpu, instr->rs);
			break;
		case OP_JIALC :
			compact = 1;
			gpr_write_32(cpu, REG_RA, cpu->PC + 4);
			cpu->PC = gpr_read_32(cpu, instr->rt) + (uint32_t) instr->offset;
			break;
		case OP_JIC :
			compact = 1;
			cpu->PC = gpr_read_32(cpu, instr->rt) + (uint32_t) instr->offset;
			break;
		case OP_LB :
			address = gpr_read_32(cpu, instr->base) + (uint32_t) instr->offset;
			i32 = sign_extend(memory_read_8(cpu, address), 8);
			gpr_write_32(cpu, instr->rt, (uint32_t) i32);
			break;
		case OP_LBU :
			address = gpr_read_32(cpu, instr->base) + (uint32_t) instr->offset;
			u8 = memory_read_8(cpu, address);
			gpr_write_32(cpu, instr->rt, (uint32_t) u8);
			break;
		case OP_LDC1 :
			address = gpr_read_32(cpu, instr->base) + (uint32_t) instr->offset;
			u64 = memory_read_64(cpu, address);
			fpr_write_64(cpu, instr->ft, u64);
			break;
		case OP_LH :
			address = gpr_read_32(cpu, instr->base) + (uint32_t) instr->offset;
			i32 = sign_extend(memory_read_16(cpu, address), 16);
			gpr_write_32(cpu, instr->rt, (uint32_t) i32);
			break;
		case OP_LHU :
			address = gpr_read_32(cpu, instr->base) + (uint32_t) instr->offset;
			u16 = memory_read_16(cpu, address);
			gpr_write_32(cpu, instr->rt, (uint32_t) u16);
			break;
		case OP_LW :
			address = gpr_read_32(cpu, instr->base) + (uint32_t) instr->offset;
			u32 = memory_read_32(cpu, address);
			gpr_write_32(cpu, instr->rt, u32);
			break;
		case OP_LWC1 :
			address = gpr_read_32(cpu, instr->base) + (uint32_t) instr->offset;
			u32 = memory_read_32(cpu, address);
			fpr_write_32(cpu, instr->ft, u32);
			break;
		case OP_LWPC :
			address = cpu->PC + ((uint32_t) instr->offset << 2);
			u32 = memory_read_32(cpu, address);
			gpr_write_32(cpu, instr->rs, u32);
			break;
		case OP_MFC1 :
			gpr_write_32(cpu, instr->rt, fpr_read_32(cpu, instr->fs));
			break;
		case OP_MFHC1 :
			gpr_write_32(cpu, instr->rt, (uint32_t) (fpr_read_64(cpu, instr->fs) >> 32));
			break;
		case OP_MOD :
			dividend = (int32_t) gpr_read_32(cpu, instr->rs);
			divisor = (int32_t) gpr_read_32(cpu, instr->rt);
			/* Arbitrary choice, MIPS32 documentation :
			'If the divisor in GPR rt is zero, the arithmetic result value is UNPREDICTABLE' */
			if(divisor == 0) i32 = dividend;
			/* Used to cover a C edge-case of integer overflow - not MIPS related */
			else if(divisor == -1 && dividend == (int32_t) 0x80000000) i32 = 0;
			else i32 = dividend % divisor;
			gpr_write_32(cpu, instr->rd, (uint32_t) i32);
			break;
		case OP_MODU :
			raw_dividend = gpr_read_32(cpu, instr->rs);
			raw_divisor = gpr_read_32(cpu, instr->rt);
			/* Arbitrary choice, MIPS32 documentation :
			'If the divisor in GPR rt is zero, the arithmetic result value is UNPREDICTABLE' */
			if(raw_divisor == 0) u32 = raw_dividend;
			/* No integer overflow with unsigned instruction */
			else u32 = raw_dividend % raw_divisor;
			gpr_write_32(cpu, instr->rd, u32);
			break;
		case OP_MOV_D :
			fpr_write_64(cpu, instr->fd, fpr_read_64(cpu, instr->fs));
			break;
		case OP_MOV_S :
			fpr_write_32(cpu, instr->fd, fpr_read_32(cpu, instr->fs));
			break;
		case OP_MTC1 :
			fpr_write_32(cpu, instr->fs, gpr_read_32(cpu, instr->rt));
			break;
		case OP_MTHC1 :
			u64 = ((uint64_t) gpr_read_32(cpu, instr->rt) << 32) | fpr_read_32(cpu, instr->fs);
			fpr_write_64(cpu, instr->fs, u64);
			break;
		case OP_MUH :
			i64 = (int64_t) (int32_t) gpr_read_32(cpu, instr->rs) * (int64_t) (int32_t) gpr_read_32(cpu, instr->rt);
			gpr_write_32(cpu, instr->rd, (uint32_t) (i64 >> 32));
			break;
		case OP_MUHU :
			u64 = (uint64_t) gpr_read_32(cpu, instr->rs) * (uint64_t) gpr_read_32(cpu, instr->rt);
			gpr_write_32(cpu, instr->rd, (uint32_t) (u64 >> 32));
			break;
		case OP_MUL :
			i64 = (int64_t) (int32_t) gpr_read_32(cpu, instr->rs) * (int64_t) (int32_t) gpr_read_32(cpu, instr->rt);
			gpr_write_32(cpu, instr->rd, (uint32_t) i64);
			break;
		case OP_MULU :
			u64 = (uint64_t) gpr_read_32(cpu, instr->rs) * (uint64_t) gpr_read_32(cpu, instr->rt);
			gpr_write_32(cpu, instr->rd, (uint32_t) u64);
			break;
		case OP_MUL_D :
			double_a = fpr_read_double(cpu, instr->fs);
			double_b = fpr_read_double(cpu, instr->ft);
			fpr_write_double(cpu, instr->fd, fp_result_double(double_a * double_b, double_a, double_b));
			break;
		case OP_MUL_S :
			single_a = fpr_read_float(cpu, instr->fs);
			single_b = fpr_read_float(cpu, instr->ft);
			fpr_write_float(cpu, instr->fd, fp_result_float(single_a * single_b, single_a, single_b));
			break;
		case OP_NAL :
			gpr_write_32(cpu, REG_RA, cpu->PC + 8);
			break;
		case OP_NEG_D :
			u64 = fpr_read_64(cpu, instr->fs) ^ 0x8000000000000000u;
			fpr_write_64(cpu, instr->fd, u64);
			break;
		case OP_NEG_S :
			u32 = fpr_read_32(cpu, instr->fs) ^ 0x80000000u;
			fpr_write_32(cpu, instr->fd, u32);
			break;
		case OP_NOP :
			break;
		case OP_NOR :
			u32 = ~(gpr_read_32(cpu, instr->rs) | gpr_read_32(cpu, instr->rt));
			gpr_write_32(cpu, instr->rd, u32);
			break;
		case OP_OR :
			u32 = gpr_read_32(cpu, instr->rs) | gpr_read_32(cpu, instr->rt);
			gpr_write_32(cpu, instr->rd, u32);
			break;
		case OP_ORI :
			u32 = gpr_read_32(cpu, instr->rs) | ((uint32_t) instr->immediate & 0xFFFF);
			gpr_write_32(cpu, instr->rt, u32);
			break;
		case OP_ROTR :
			u32 = gpr_read_32(cpu, instr->rt);
			u32 = (u32 >> instr->sa) | (u32 << ((32 - instr->sa) & 31));
			gpr_write_32(cpu, instr->rd, u32);
			break;
		case OP_ROTRV :
			u8 = (uint8_t) (gpr_read_32(cpu, instr->rs) & 0x1F);
			u32 = gpr_read_32(cpu, instr->rt);
			u32 = u32 >> u8 | u32 << ((32 - u8) & 31);
			gpr_write_32(cpu, instr->rd, u32);
			break;
		case OP_SB :
			address = gpr_read_32(cpu, instr->base) + (uint32_t) instr->offset;
			memory_write_8(cpu, address, (uint8_t) gpr_read_32(cpu, instr->rt));
			break;
		case OP_SDC1 :
			address = gpr_read_32(cpu, instr->base) + (uint32_t) instr->offset;
			memory_write_64(cpu, address, fpr_read_64(cpu, instr->ft));
			break;
		case OP_SEB :
			i32 = sign_extend((int32_t) gpr_read_32(cpu, instr->rt), 8);
			gpr_write_32(cpu, instr->rd, (uint32_t) i32);
			break;
		case OP_SEH :
			i32 = sign_extend((int32_t) gpr_read_32(cpu, instr->rt), 16);
			gpr_write_32(cpu, instr->rd, (uint32_t) i32);
			break;
		case OP_SELEQZ :
			if((int32_t) gpr_read_32(cpu, instr->rt) == 0) gpr_write_32(cpu, instr->rd, gpr_read_32(cpu, instr->rs));
			else gpr_write_32(cpu, instr->rd, 0);
			break;
		case OP_SELNEZ :
			if((int32_t) gpr_read_32(cpu, instr->rt) != 0) gpr_write_32(cpu, instr->rd, gpr_read_32(cpu, instr->rs));
			else gpr_write_32(cpu, instr->rd, 0);
			break;
		case OP_SH :
			address = gpr_read_32(cpu, instr->base) + (uint32_t) instr->offset;
			memory_write_16(cpu, address, (uint16_t) gpr_read_32(cpu, instr->rt));
			break;
		case OP_SLL :
			u32 = gpr_read_32(cpu, instr->rt) << instr->sa;
			gpr_write_32(cpu, instr->rd, u32);
			break;
		case OP_SLLV :
			u8 = (uint8_t) (gpr_read_32(cpu, instr->rs) & 0x1F);
			u32 = gpr_read_32(cpu, instr->rt) << u8;
			gpr_write_32(cpu, instr->rd, u32);
			break;
		case OP_SLT :
			gpr_write_32(cpu, instr->rd, (int32_t) gpr_read_32(cpu, instr->rs) < (int32_t) gpr_read_32(cpu, instr->rt));
			break;
		case OP_SLTI :
			gpr_write_32(cpu, instr->rt, (int32_t) gpr_read_32(cpu, instr->rs) < instr->immediate);
			break;
		case OP_SLTIU :
			gpr_write_32(cpu, instr->rt, gpr_read_32(cpu, instr->rs) < (uint32_t) instr->immediate);
			break;
		case OP_SLTU :
			gpr_write_32(cpu, instr->rd, gpr_read_32(cpu, instr->rs) < gpr_read_32(cpu, instr->rt));
			break;
		case OP_SRA :
			/* Implementation-defined for arithmetic shift :
			guaranteed by gcc/clang, not C itself */
			i32 = ((int32_t) gpr_read_32(cpu, instr->rt)) >> instr->sa;
			gpr_write_32(cpu, instr->rd, (uint32_t) i32);
			break;
		case OP_SRAV :
			u8 = (uint8_t) (gpr_read_32(cpu, instr->rs) & 0x1F);
			i32 = ((int32_t) gpr_read_32(cpu, instr->rt)) >> u8;
			gpr_write_32(cpu, instr->rd, (uint32_t) i32);
			break;
		case OP_SRL :
			u32 = gpr_read_32(cpu, instr->rt) >> instr->sa;
			gpr_write_32(cpu, instr->rd, u32);
			break;
		case OP_SRLV :
			u8 = (uint8_t) (gpr_read_32(cpu, instr->rs) & 0x1F);
			u32 = gpr_read_32(cpu, instr->rt) >> u8;
			gpr_write_32(cpu, instr->rd, u32);
			break;
		case OP_SUB :
			i64 = (int64_t) (int32_t) gpr_read_32(cpu, instr->rs) - (int64_t) (int32_t) gpr_read_32(cpu, instr->rt);
			if(i64 > INT32_MAX || i64 < INT32_MIN) fprintf(stderr, "[!] Exception : Integer Overflow\n");
			else gpr_write_32(cpu, instr->rd, (uint32_t) i64);
			break;
		case OP_SUBU :
			u32 = gpr_read_32(cpu, instr->rs) - gpr_read_32(cpu, instr->rt);
			gpr_write_32(cpu, instr->rd, u32);
			break;
		case OP_SUB_D :
			double_a = fpr_read_double(cpu, instr->fs);
			double_b = fpr_read_double(cpu, instr->ft);
			fpr_write_double(cpu, instr->fd, fp_result_double(double_a - double_b, double_a, double_b));
			break;
		case OP_SUB_S :
			single_a = fpr_read_float(cpu, instr->fs);
			single_b = fpr_read_float(cpu, instr->ft);
			fpr_write_float(cpu, instr->fd, fp_result_float(single_a - single_b, single_a, single_b));
			break;
		case OP_SW :
			u32 = gpr_read_32(cpu, instr->rt);
			address = gpr_read_32(cpu, instr->base) + (uint32_t) instr->offset;
			memory_write_32(cpu, address, u32);
			break;
		case OP_SWC1 :
			address = gpr_read_32(cpu, instr->base) + (uint32_t) instr->offset;
			memory_write_32(cpu, address, fpr_read_32(cpu, instr->ft));
			break;
		case OP_XOR :
			u32 = gpr_read_32(cpu, instr->rs) ^ gpr_read_32(cpu, instr->rt);
			gpr_write_32(cpu, instr->rd, u32);
			break;
		case OP_XORI :
			u32 = gpr_read_32(cpu, instr->rs) ^ ((uint32_t) instr->immediate & 0xFFFF);
			gpr_write_32(cpu, instr->rt, u32);
			break;
		default :
			break;
	}

	/* In MIPS32 Release 6, compact branches skip delay slot
	   This way we ensure this kind of instruction behaves properly */
	if(!compact) cpu->PC += 4;

	if(pending_PC != INVALID_PC) cpu->PC = pending_PC;

	if(cfg->verbose) printf("[#] Executed : %s", instr->to_string);
}
