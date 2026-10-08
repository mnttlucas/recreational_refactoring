#include <stdint.h>
#include <stdio.h>

#include "config.h"
#include "cpu.h"
#include "execute.h"
#include "mips_memory.h"
#include "mips_registers.h"
#include "utils.h"

void execute_instruction(CPU *cpu, Config *cfg, instruction instr)
{
	int32_t dividend, divisor, i32;
	int64_t i64;
	uint16_t u16;
	uint32_t address, pending_PC, raw_dividend, raw_divisor, u32;
	uint64_t u64;
	uint8_t compact = 0, u8;

	pending_PC = cpu->next_PC;
	cpu->next_PC = INVALID_PC;
	
	switch(instr.opcode)
	{
		case ADD :
			i64 = (int64_t) (int32_t) gpr_read_32(cpu, instr.rs) + (int64_t) (int32_t) gpr_read_32(cpu, instr.rt);
			if(i64 > INT32_MAX || i64 < INT32_MIN) fprintf(stderr, "[!] Exception : Integer Overflow\n");
			else gpr_write_32(cpu, instr.rd, (uint32_t) i64);
			break;
		case ADDIU :
			u32 = gpr_read_32(cpu, instr.rs) + (uint32_t) instr.immediate;
			gpr_write_32(cpu, instr.rt, u32);
			break;
		case ADDIUPC :
			u32 = cpu->PC + (uint32_t) instr.immediate;
			gpr_write_32(cpu, instr.rs, u32);
			break;
		case ADDU :
			u32 = gpr_read_32(cpu, instr.rs) + gpr_read_32(cpu, instr.rt);
			gpr_write_32(cpu, instr.rd, u32);
			break;
		case ALUIPC :
			u32 = ~0xFFFFu & (cpu->PC + ((uint32_t) instr.immediate << 16));
			gpr_write_32(cpu, instr.rs, u32);
			break;
		case AND :
			u32 = gpr_read_32(cpu, instr.rs) & gpr_read_32(cpu, instr.rt);
			gpr_write_32(cpu, instr.rd, u32);
			break;
		case ANDI :
			u32 = gpr_read_32(cpu, instr.rs) & ((uint32_t) instr.immediate & 0xFFFF);
			gpr_write_32(cpu, instr.rt, u32);
			break;
		case AUI :
			u32 = gpr_read_32(cpu, instr.rs) + ((uint32_t) instr.immediate << 16);
			gpr_write_32(cpu, instr.rt, u32);
			break;
		case AUIPC :
			u32 = cpu->PC + ((uint32_t) instr.immediate << 16);
			gpr_write_32(cpu, instr.rs, u32);
			break;
		case BAL :
			gpr_write_32(cpu, REG_RA, cpu->PC + 8);
			cpu->next_PC = cpu->PC + ((uint32_t) instr.offset << 2) + 4;
			break;
		case BALC :
			compact = 1;
			gpr_write_32(cpu, REG_RA, cpu->PC + 4);
			cpu->PC += ((uint32_t) instr.offset << 2) + 4;
			break;
		case BC :
			compact = 1;
			cpu->PC += ((uint32_t) instr.offset << 2) + 4;
			break;
		case BEQ :
			if(gpr_read_32(cpu, instr.rs) == gpr_read_32(cpu, instr.rt))
				cpu->next_PC = cpu->PC + ((uint32_t) instr.offset << 2) + 4;
			break;
		case BEQC :
			if(gpr_read_32(cpu, instr.rs) == gpr_read_32(cpu, instr.rt))
			{
				compact = 1;
				cpu->PC += ((uint32_t) instr.offset << 2) + 4;
			}
			break;
		case BEQZALC :
			if(gpr_read_32(cpu, instr.rt) == 0)
			{
				compact = 1;
				gpr_write_32(cpu, REG_RA, cpu->PC + 4);
				cpu->PC += ((uint32_t) instr.offset << 2) + 4;
			}
			break;
		case BEQZC :
			if(gpr_read_32(cpu, instr.rs) == 0)
			{
				compact = 1;
				cpu->PC += ((uint32_t) instr.offset << 2) + 4;
			}
			break;
		case BGEC :
			if((int32_t) gpr_read_32(cpu, instr.rs) >= (int32_t) gpr_read_32(cpu, instr.rt))
			{
				compact = 1;
				cpu->PC += ((uint32_t) instr.offset << 2) + 4;
			}
			break;
		case BGEUC :
			if(gpr_read_32(cpu, instr.rs) >= gpr_read_32(cpu, instr.rt))
			{
				compact = 1;
				cpu->PC += ((uint32_t) instr.offset << 2) + 4;
			}
			break;
		case BGEZ :
			if((int32_t) gpr_read_32(cpu, instr.rs) >= 0)
				cpu->next_PC = cpu->PC + ((uint32_t) instr.offset << 2) + 4;
			break;
		case BGEZALC :
			if((int32_t) gpr_read_32(cpu, instr.rt) >= 0)
			{
				compact = 1;
				gpr_write_32(cpu, REG_RA, cpu->PC + 4);
				cpu->PC += ((uint32_t) instr.offset << 2) + 4;
			}
			break;
		case BGEZC :
			if((int32_t) gpr_read_32(cpu, instr.rt) >= 0)
			{
				compact = 1;
				cpu->PC += ((uint32_t) instr.offset << 2) + 4;
			}
			break;
		case BGTZ :
			if((int32_t) gpr_read_32(cpu, instr.rs) > 0)
				cpu->next_PC = cpu->PC + ((uint32_t) instr.offset << 2) + 4;
			break;
		case BGTZALC :
			if((int32_t) gpr_read_32(cpu, instr.rt) > 0)
			{
				compact = 1;
				gpr_write_32(cpu, REG_RA, cpu->PC + 4);
				cpu->PC += ((uint32_t) instr.offset << 2) + 4;
			}
			break;
		case BGTZC :
			if((int32_t) gpr_read_32(cpu, instr.rt) > 0)
			{
				compact = 1;
				cpu->PC += ((uint32_t) instr.offset << 2) + 4;
			}
			break;
		case BITSWAP :
			/* Optimized algorithm I thought was interesting */
			u32 = gpr_read_32(cpu, instr.rt);
			u32 = ((u32 >> 1) & 0x55555555) | ((u32 & 0x55555555) << 1);
			u32 = ((u32 >> 2) & 0x33333333) | ((u32 & 0x33333333) << 2);
			u32 = ((u32 >> 4) & 0x0F0F0F0F) | ((u32 & 0x0F0F0F0F) << 4);
			gpr_write_32(cpu, instr.rd, u32);
			break;
		case BLEZ :
			if((int32_t) gpr_read_32(cpu, instr.rs) <= 0)
				cpu->next_PC = cpu->PC + ((uint32_t) instr.offset << 2) + 4;
			break;
		case BLEZALC :
			if((int32_t) gpr_read_32(cpu, instr.rt) <= 0)
			{
				compact = 1;
				gpr_write_32(cpu, REG_RA, cpu->PC + 4);
				cpu->PC += ((uint32_t) instr.offset << 2) + 4;
			}
			break;
		case BLEZC :
			if((int32_t) gpr_read_32(cpu, instr.rt) <= 0)
			{
				compact = 1;
				cpu->PC += ((uint32_t) instr.offset << 2) + 4;
			}
			break;
		case BLTC :
			if((int32_t) gpr_read_32(cpu, instr.rs) < (int32_t) gpr_read_32(cpu, instr.rt))
			{
				compact = 1;
				cpu->PC += ((uint32_t) instr.offset << 2) + 4;
			}
			break;
		case BLTUC :
			if(gpr_read_32(cpu, instr.rs) < gpr_read_32(cpu, instr.rt))
			{
				compact = 1;
				cpu->PC += ((uint32_t) instr.offset << 2) + 4;
			}
			break;
		case BLTZ :
			if((int32_t) gpr_read_32(cpu, instr.rs) < 0)
				cpu->next_PC = cpu->PC + ((uint32_t) instr.offset << 2) + 4;
			break;
		case BLTZALC :
			if((int32_t) gpr_read_32(cpu, instr.rt) < 0)
			{
				compact = 1;
				gpr_write_32(cpu, REG_RA, cpu->PC + 4);
				cpu->PC += ((uint32_t) instr.offset << 2) + 4;
			}
			break;
		case BLTZC :
			if((int32_t) gpr_read_32(cpu, instr.rt) < 0)
			{
				compact = 1;
				cpu->PC += ((uint32_t) instr.offset << 2) + 4;
			}
			break;
		case BNE :
			if(gpr_read_32(cpu, instr.rs) != gpr_read_32(cpu, instr.rt))
				cpu->next_PC = cpu->PC + ((uint32_t) instr.offset << 2) + 4;
			break;
		case BNEC :
			if(gpr_read_32(cpu, instr.rs) != gpr_read_32(cpu, instr.rt))
			{
				compact = 1;
				cpu->PC += ((uint32_t) instr.offset << 2) + 4;
			}
			break;
		case BNEZALC :
			if((int32_t) gpr_read_32(cpu, instr.rt) != 0)
			{
				compact = 1;
				gpr_write_32(cpu, REG_RA, cpu->PC + 4);
				cpu->PC += ((uint32_t) instr.offset << 2) + 4;
			}
			break;
		case BNEZC :
			if((int32_t) gpr_read_32(cpu, instr.rs) != 0)
			{
				compact = 1;
				cpu->PC += ((uint32_t) instr.offset << 2) + 4;
			}
			break;
		case BNVC :
			i64 = (int64_t) (int32_t) gpr_read_32(cpu, instr.rs) + (int64_t) (int32_t) gpr_read_32(cpu, instr.rt);
			if(i64 <= INT32_MAX && i64 >= INT32_MIN)
			{
				compact = 1;
				cpu->PC += ((uint32_t) instr.offset << 2) + 4;
			}
			break;
		case BOVC :
			i64 = (int64_t) (int32_t) gpr_read_32(cpu, instr.rs) + (int64_t) (int32_t) gpr_read_32(cpu, instr.rt);
			if(i64 > INT32_MAX || i64 < INT32_MIN)
			{
				compact = 1;
				cpu->PC += ((uint32_t) instr.offset << 2) + 4;
			}
			break;
		case CLO :
			/* Naive CLO/Z implementation, didn't want to just
			use C __builtin_clz/popu8 */
			u32 = gpr_read_32(cpu, instr.rs);
			if(u32 == 0xFFFFFFFFu) gpr_write_32(cpu, instr.rd, 32);
			else
			{
				u8 = 0;
				while(u32 & 0x80000000)
				{
					u32 <<= 1;
					u8++; 
				}
				gpr_write_32(cpu, instr.rd, (uint32_t) u8);
			}
			break;
		case CLZ :
			u32 = gpr_read_32(cpu, instr.rs);
			if(u32 == 0u) gpr_write_32(cpu, instr.rd, 32);
			else
			{
				u8 = 0;
				while(!(u32 & 0x80000000))
				{
					u32 <<= 1;
					u8++; 
				}
				gpr_write_32(cpu, instr.rd, (uint32_t) u8);
			}
			break;
		case DIV :
			dividend = (int32_t) gpr_read_32(cpu, instr.rs);
			divisor = (int32_t) gpr_read_32(cpu, instr.rt);
			/* Arbitrary choice, MIPS32 documentation :
			'If the divisor in GPR rt is zero, the arithmetic result value is UNPREDICTABLE' */
			if(divisor == 0) i32 = divisor;
			/* Used to cover a C edge-case of integer overflow - not MIPS related */
			else if(divisor == -1 && dividend == (int32_t) 0x80000000) i32 = dividend;
			else i32 = dividend / divisor;
			gpr_write_32(cpu, instr.rd, (uint32_t) i32);
			break;
		case DIVU :
			raw_dividend = gpr_read_32(cpu, instr.rs);
			raw_divisor = gpr_read_32(cpu, instr.rt);
			/* Arbitrary choice, MIPS32 documentation :
			'If the divisor in GPR rt is zero, the arithmetic result value is UNPREDICTABLE' */
			if(raw_divisor == 0) u32 = raw_divisor;
			/* No integer overflow with unsigned instruction */
			else u32 = raw_dividend / raw_divisor;
			gpr_write_32(cpu, instr.rd, u32);
			break;
		case J :
			cpu->next_PC = ((cpu->PC + 4) & 0xF0000000u) | ((uint32_t) (instr.target << 2) & 0x0FFFFFFCu);
			break;
		case JAL :
			gpr_write_32(cpu, REG_RA, cpu->PC + 8);
			cpu->next_PC = ((cpu->PC + 4) & 0xF0000000u) | ((uint32_t) (instr.target << 2) & 0x0FFFFFFCu);
			break;
		case JALR :
			gpr_write_32(cpu, instr.rd, cpu->PC + 8);
			if(gpr_read_32(cpu, instr.rs) & 0x03u) fprintf(stderr, "[!] Exception : Address Error\n");
			else cpu->next_PC = gpr_read_32(cpu, instr.rs);
			break;
		case JIALC :
			compact = 1;
			gpr_write_32(cpu, REG_RA, cpu->PC + 4);
			cpu->PC = gpr_read_32(cpu, instr.rt) + ((uint32_t) instr.offset << 2);
			break;
		case JIC :
			compact = 1;
			cpu->PC = gpr_read_32(cpu, instr.rt) + ((uint32_t) instr.offset << 2);
			break;
		case LB :
			address = gpr_read_32(cpu, instr.base) + (uint32_t) instr.offset;
			i32 = sign_extend(memory_read_8(cpu, address), 8);
			gpr_write_32(cpu, instr.rt, (uint32_t) i32);
			break;
		case LBU :
			address = gpr_read_32(cpu, instr.base) + (uint32_t) instr.offset;
			u8 = memory_read_8(cpu, address);
			gpr_write_32(cpu, instr.rt, (uint32_t) u8);
			break;
		case LH :
			address = gpr_read_32(cpu, instr.base) + (uint32_t) instr.offset;
			i32 = sign_extend(memory_read_16(cpu, address), 16);
			gpr_write_32(cpu, instr.rt, (uint32_t) i32);
			break;
		case LHU :
			address = gpr_read_32(cpu, instr.base) + (uint32_t) instr.offset;
			u16 = memory_read_16(cpu, address);
			gpr_write_32(cpu, instr.rt, (uint32_t) u16);
			break;
		case LW :
			address = gpr_read_32(cpu, instr.base) + (uint32_t) instr.offset;
			u32 = memory_read_32(cpu, address);
			gpr_write_32(cpu, instr.rt, u32);
			break;
		case LWPC :
			address = cpu->PC + (uint32_t) (instr.offset << 2);
			u32 = memory_read_32(cpu, address);
			gpr_write_32(cpu, instr.rs, u32);
			break;
		case MOD :
			dividend = (int32_t) gpr_read_32(cpu, instr.rs);
			divisor = (int32_t) gpr_read_32(cpu, instr.rt);
			/* Arbitrary choice, MIPS32 documentation :
			'If the divisor in GPR rt is zero, the arithmetic result value is UNPREDICTABLE' */
			if(divisor == 0) i32 = dividend;
			/* Used to cover a C edge-case of integer overflow - not MIPS related */
			else if(divisor == -1 && dividend == (int32_t) 0x80000000) i32 = 0;
			else i32 = dividend % divisor;
			gpr_write_32(cpu, instr.rd, (uint32_t) i32);
			break;
		case MODU :
			raw_dividend = gpr_read_32(cpu, instr.rs);
			raw_divisor = gpr_read_32(cpu, instr.rt);
			/* Arbitrary choice, MIPS32 documentation :
			'If the divisor in GPR rt is zero, the arithmetic result value is UNPREDICTABLE' */
			if(raw_divisor == 0) u32 = raw_dividend;
			/* No integer overflow with unsigned instruction */
			else u32 = raw_dividend % raw_divisor;
			gpr_write_32(cpu, instr.rd, u32);
			break;
		case MUH :
			i64 = (int64_t) (int32_t) gpr_read_32(cpu, instr.rs) * (int64_t) (int32_t) gpr_read_32(cpu, instr.rt);
			gpr_write_32(cpu, instr.rd, (uint32_t) (i64 >> 32));
			break;
		case MUHU :
			u64 = (uint64_t) gpr_read_32(cpu, instr.rs) * (uint64_t) gpr_read_32(cpu, instr.rt);
			gpr_write_32(cpu, instr.rd, (uint32_t) (u64 >> 32));
			break;
		case MUL :
			i64 = (int64_t) (int32_t) gpr_read_32(cpu, instr.rs) * (int64_t) (int32_t) gpr_read_32(cpu, instr.rt);
			gpr_write_32(cpu, instr.rd, (uint32_t) i64);
			break;
		case MULU :
			u64 = (uint64_t) gpr_read_32(cpu, instr.rs) * (uint64_t) gpr_read_32(cpu, instr.rt);
			gpr_write_32(cpu, instr.rd, (uint32_t) u64);
			break;
		case NAL :
			gpr_write_32(cpu, REG_RA, cpu->PC + 8);
			break;
		case NOP :
			break;
		case NOR :
			u32 = ~(gpr_read_32(cpu, instr.rs) | gpr_read_32(cpu, instr.rt));
			gpr_write_32(cpu, instr.rd, u32);
			break;
		case OR :
			u32 = gpr_read_32(cpu, instr.rs) | gpr_read_32(cpu, instr.rt);
			gpr_write_32(cpu, instr.rd, u32);
			break;
		case ORI :
			u32 = gpr_read_32(cpu, instr.rs) | ((uint32_t) instr.immediate & 0xFFFF);
			gpr_write_32(cpu, instr.rt, u32);
			break;
		case ROTR :
			u32 = gpr_read_32(cpu, instr.rt);
			u32 = (u32 >> instr.sa) | (u32 << ((32 - instr.sa) & 31));
			gpr_write_32(cpu, instr.rd, u32);
			break;
		case ROTRV :
			u8 = (uint8_t) (gpr_read_32(cpu, instr.rs) & 0x1F);
			u32 = gpr_read_32(cpu, instr.rt);
			u32 = u32 >> u8 | u32 << ((32 - u8) & 31);
			gpr_write_32(cpu, instr.rd, u32);
			break;
		case SB :
			address = gpr_read_32(cpu, instr.base) + (uint32_t) instr.offset;
			memory_write_8(cpu, address, (uint8_t) gpr_read_32(cpu, instr.rt));
			break;
		case SEB :
			i32 = sign_extend((int32_t) gpr_read_32(cpu, instr.rt), 8);
			gpr_write_32(cpu, instr.rd, (uint32_t) i32);
			break;
		case SEH :
			i32 = sign_extend((int32_t) gpr_read_32(cpu, instr.rt), 16);
			gpr_write_32(cpu, instr.rd, (uint32_t) i32);
			break;
		case SELEQZ :
			if((int32_t) gpr_read_32(cpu, instr.rt) == 0) gpr_write_32(cpu, instr.rd, gpr_read_32(cpu, instr.rs));
			else gpr_write_32(cpu, instr.rd, 0);
			break;
		case SELNEZ :
			if((int32_t) gpr_read_32(cpu, instr.rt) != 0) gpr_write_32(cpu, instr.rd, gpr_read_32(cpu, instr.rs));
			else gpr_write_32(cpu, instr.rd, 0);
			break;
		case SH :
			address = gpr_read_32(cpu, instr.base) + (uint32_t) instr.offset;
			memory_write_16(cpu, address, (uint16_t) gpr_read_32(cpu, instr.rt));
			break;
		case SLL :
			u32 = gpr_read_32(cpu, instr.rt) << instr.sa;
			gpr_write_32(cpu, instr.rd, u32);
			break;
		case SLLV :
			u8 = (uint8_t) (gpr_read_32(cpu, instr.rs) & 0x1F);
			u32 = gpr_read_32(cpu, instr.rt) << u8;
			gpr_write_32(cpu, instr.rd, u32);
			break;
		case SLT :
			gpr_write_32(cpu, instr.rd, (int32_t) gpr_read_32(cpu, instr.rs) < (int32_t) gpr_read_32(cpu, instr.rt));
			break;
		case SLTI :
			gpr_write_32(cpu, instr.rt, (int32_t) gpr_read_32(cpu, instr.rs) < instr.immediate);
			break;
		case SLTIU :
			gpr_write_32(cpu, instr.rt, gpr_read_32(cpu, instr.rs) < (uint32_t) instr.immediate);
			break;
		case SLTU :
			gpr_write_32(cpu, instr.rd, gpr_read_32(cpu, instr.rs) < gpr_read_32(cpu, instr.rt));
			break;
		case SRA :
			/* Implementation-defined for arithmetic shift :
			guaranteed by gcc/clang, not C itself */
			i32 = ((int32_t) gpr_read_32(cpu, instr.rt)) >> instr.sa;
			gpr_write_32(cpu, instr.rd, (uint32_t) i32);
			break;
		case SRAV :
			u8 = (uint8_t) (gpr_read_32(cpu, instr.rs) & 0x1F);
			i32 = ((int32_t) gpr_read_32(cpu, instr.rt)) >> u8;
			gpr_write_32(cpu, instr.rd, (uint32_t) i32);
			break;
		case SRL :
			u32 = gpr_read_32(cpu, instr.rt) >> instr.sa;
			gpr_write_32(cpu, instr.rd, u32);
			break;
		case SRLV :
			u8 = (uint8_t) (gpr_read_32(cpu, instr.rs) & 0x1F);
			u32 = gpr_read_32(cpu, instr.rt) >> u8;
			gpr_write_32(cpu, instr.rd, u32);
			break;
		case SUB :
			i64 = (int64_t) (int32_t) gpr_read_32(cpu, instr.rs) - (int64_t) (int32_t) gpr_read_32(cpu, instr.rt);
			if(i64 > INT32_MAX || i64 < INT32_MIN) fprintf(stderr, "[!] Exception : Integer Overflow\n");
			else gpr_write_32(cpu, instr.rd, (uint32_t) i64);
			break;
		case SUBU :
			u32 = gpr_read_32(cpu, instr.rs) - gpr_read_32(cpu, instr.rt);
			gpr_write_32(cpu, instr.rd, u32);
			break;
		case SW :
			u32 = gpr_read_32(cpu, instr.rt);
			address = gpr_read_32(cpu, instr.base) + (uint32_t) instr.offset;
			memory_write_32(cpu, address, u32);
			break;
		case XOR :
			u32 = gpr_read_32(cpu, instr.rs) ^ gpr_read_32(cpu, instr.rt);
			gpr_write_32(cpu, instr.rd, u32);
			break;
		case XORI :
			u32 = gpr_read_32(cpu, instr.rs) ^ ((uint32_t) instr.immediate & 0xFFFF);
			gpr_write_32(cpu, instr.rt, u32);
			break;
		default :
			fprintf(stderr, "[!] This line was either a comment or an unknown command\n");
			break;
	}

	/* In MIPS32 Release 6, compact branches skip delay slot
	   This way we ensure this kind of instruction behaves properly */
	if(!compact) cpu->PC += 4;

	if(pending_PC != INVALID_PC) cpu->PC = pending_PC;

	if(cfg->verbose) printf("%s", instr.to_string);
}