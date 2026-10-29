#include <ctype.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decode.h"
#include "hex.h"
#include "instruction.h"
#include "mips_encoding.h"
#include "utils.h"

#define HEX_DIGITS 8
#define OPERANDS_SIZE 64

/* Parses the 32-bit hexadecimal word at the start of `text` : 0x then 1 to 8 digits, or exactly 8 digits.
   Nothing but blanks may follow it, otherwise it is not one. Returns its length (0 if there is none). */
static size_t parse_hex_word(const char *text, uint32_t *word)
{
	const int prefixed = text[0] == '0' && (text[1] == 'x' || text[1] == 'X');
	const char *digits = text + (prefixed ? 2 : 0);
	size_t count = 0;

	while(isxdigit((unsigned char) digits[count])) count++;
	if(digits[count] != '\0' && !isblank((unsigned char) digits[count])) return(0);
	if(count == 0 || count > HEX_DIGITS || (!prefixed && count != HEX_DIGITS)) return(0);
	*word = (uint32_t) strtoul(digits, NULL, 16);

	return((size_t) (digits - text) + count);
}

static unsigned extract_unsigned(uint32_t word, unsigned shift, unsigned size)
{
	return((word >> shift) & low_mask(size));
}

static int32_t extract_signed(uint32_t word, unsigned shift, uint8_t size)
{
	return(sign_extend((int32_t) extract_unsigned(word, shift, size), size));
}

/* The assembly operands of `word` seen as an instruction of this format, the exact opposite of what the assembler does.
   The result is only a candidate, the assembler decides if it really encodes `word`. Returns 0 if the format can't be tried. */
static int operands_from_word(const InstructionDesc *id, uint32_t word, char *text, size_t size)
{
	const unsigned rs = extract_unsigned(word, RS_SHIFT, REG_FIELD_SIZE), rt = extract_unsigned(word, RT_SHIFT, REG_FIELD_SIZE);
	const unsigned rd = extract_unsigned(word, RD_SHIFT, REG_FIELD_SIZE), sa = extract_unsigned(word, SHAMT_SHIFT, REG_FIELD_SIZE);
	const int32_t imm16 = extract_signed(word, 0, IMM_16_SIZE);

	switch(id->format)
	{
		case FMT_NAL :
		case FMT_NOP :
			text[0] = '\0';
			return(1);
		case FMT_CODE_16 :
			snprintf(text, size, "%u", extract_unsigned(word, 0, IMM_16_SIZE));
			return(1);
		case FMT_CODE_20 :
			snprintf(text, size, "%u", extract_unsigned(word, CODE_SHIFT, CODE_20_SIZE));
			return(1);
		case FMT_FD_FS :
			snprintf(text, size, "$f%u, $f%u", sa, rd);
			return(1);
		case FMT_FD_FS_FT :
			snprintf(text, size, "$f%u, $f%u, $f%u", sa, rd, rt);
			return(1);
		case FMT_FT_OFFSET_16 :
			snprintf(text, size, "$f%u, %d", rt, imm16);
			return(1);
		case FMT_FT_OFFSET_16_BASE :
			snprintf(text, size, "$f%u, %d($%u)", rt, imm16, rs);
			return(1);
		case FMT_OFFSET_16 :
			snprintf(text, size, "%d", imm16);
			return(1);
		case FMT_OFFSET_26 :
			snprintf(text, size, "%d", extract_signed(word, 0, OFFSET_26_SIZE));
			return(1);
		case FMT_RD_RS :
			snprintf(text, size, "$%u, $%u", rd, rs);
			return(1);
		case FMT_RD_RT :
			snprintf(text, size, "$%u, $%u", rd, rt);
			return(1);
		case FMT_RD_RS_RT :
			snprintf(text, size, "$%u, $%u, $%u", rd, rs, rt);
			return(1);
		case FMT_RD_RS_RT_BP :
			snprintf(text, size, "$%u, $%u, $%u, %u", rd, rs, rt, extract_unsigned(word, SHAMT_SHIFT, BP_FIELD_SIZE));
			return(1);
		case FMT_RD_RS_RT_SA :
			snprintf(text, size, "$%u, $%u, $%u, %u", rd, rs, rt, extract_unsigned(word, SHAMT_SHIFT, BP_FIELD_SIZE) + 1);
			return(1);
		case FMT_RD_RT_RS :
			snprintf(text, size, "$%u, $%u, $%u", rd, rt, rs);
			return(1);
		case FMT_RD_RT_SA :
			snprintf(text, size, "$%u, $%u, %u", rd, rt, sa);
			return(1);
		case FMT_RS :
			snprintf(text, size, "$%u", rs);
			return(1);
		case FMT_RS_IMMEDIATE_16 :
		case FMT_RS_OFFSET_16 :
			snprintf(text, size, "$%u, %d", rs, imm16);
			return(1);
		case FMT_RS_IMMEDIATE_19 :
		case FMT_RS_OFFSET_19 :
			snprintf(text, size, "$%u, %d", rs, extract_signed(word, 0, OFFSET_19_SIZE));
			return(1);
		case FMT_RS_OFFSET_21 :
			snprintf(text, size, "$%u, %d", rs, extract_signed(word, 0, OFFSET_21_SIZE));
			return(1);
		case FMT_RS_RT :
			snprintf(text, size, "$%u, $%u", rs, rt);
			return(1);
		case FMT_RS_RT_OFFSET_16 :
			snprintf(text, size, "$%u, $%u, %d", rs, rt, imm16);
			return(1);
		case FMT_RT_FS :
			snprintf(text, size, "$%u, $f%u", rt, rd);
			return(1);
		case FMT_RT_IMMEDIATE_16 :
		case FMT_RT_OFFSET_16 :
			snprintf(text, size, "$%u, %d", rt, imm16);
			return(1);
		case FMT_RT_OFFSET_BASE :
			snprintf(text, size, "$%u, %d($%u)", rt, imm16, rs);
			return(1);
		case FMT_RT_RS_IMMEDIATE_16 :
			if(decode_immediate_range(id->op) == RANGE_UNSIGNED) snprintf(text, size, "$%u, $%u, %u", rt, rs, extract_unsigned(word, 0, IMM_16_SIZE));
			else snprintf(text, size, "$%u, $%u, %d", rt, rs, imm16);
			return(1);
		case FMT_RT_RS_POS_SIZE :
			/* EXT : msbd = size - 1, INS : msb = pos + size - 1, and lsb = pos in both cases (msb or msbd is in the rd position) */
			if(id->op == OP_INS)
			{
				if(rd < sa) return(0);
				snprintf(text, size, "$%u, $%u, %u, %u", rt, rs, sa, rd - sa + 1);
			}
			else snprintf(text, size, "$%u, $%u, %u, %u", rt, rs, sa, rd + 1);
			return(1);
		case FMT_RT_RS_RT :
			snprintf(text, size, "$%u, $%u, $%u", rt, rs, rt);
			return(1);
		case FMT_TARGET :
			snprintf(text, size, "%u", extract_unsigned(word, 0, OFFSET_26_SIZE));
			return(1);
		default :
			/* FMT_EXIT is not an instruction, FMT_RT_RS_OFFSET_16 only holds assembler aliases (BGTC, BLEC...) of other entries */
			return(0);
	}
}

/* The code field of the trap instructions is ignored by the hardware, it can hold anything */
static uint32_t ignored_bits(Opcode op)
{
	switch(op)
	{
		case OP_TEQ :
		case OP_TGE :
		case OP_TGEU :
		case OP_TLT :
		case OP_TLTU :
		case OP_TNE :
			return(TRAP_CODE_MASK);
		default :
			return(0);
	}
}

/* There is no table of the encodings : each entry of the instruction table is asked to assemble the operands that
   `word` would have in its format, and the first one that gives `word` back is the instruction. Encodings that
   are not exactly an assembled one (reserved fields not zero, rs = rt for BEQC...) are therefore not decoded. */
static void decode_word(uint32_t word, Instruction *instr)
{
	char operands[OPERANDS_SIZE];

	decode_set_quiet(1);
	for(size_t i = 0; instruction_at(i); i++)
	{
		const InstructionDesc *id = instruction_at(i);
		Instruction candidate = {0};

		if(!operands_from_word(id, word, operands, sizeof(operands))) continue;
		decode_assemble(id, operands, &candidate);
		if(candidate.error == NO_ERR && ((candidate.encoding ^ word) & ~ignored_bits(id->op)) == 0)
		{
			/* Keep the word as it was written, the code field of a trap included */
			candidate.encoding = word;
			*instr = candidate;
			decode_set_quiet(0);
			return;
		}
	}
	decode_set_quiet(0);

	/* Not through raise_exception() : the word is rejected when the program is loaded, nothing is executed */
	fprintf(stderr, "[!] Reserved Instruction : 0x%08" PRIx32 " is not an implemented instruction\n", word);
	instr->error |= ERR_RESERVED;
}

int hex_is_word(const char *text)
{
	uint32_t word;

	return(parse_hex_word(text, &word) != 0);
}

void hex_decode_line(const char *text, Instruction *instr)
{
	uint32_t word;
	const size_t length = parse_hex_word(text, &word);

	if(!length)
	{
		fprintf(stderr, "[!] Hexadecimal word expected : %s\n", text);
		instr->error |= ERR_SYNTAX;
		return;
	}

	const char *rest = text + length;
	while(isblank((unsigned char) *rest)) rest++;
	if(*rest != '\0')
	{
		fprintf(stderr, "[!] Syntax error : nothing is allowed after a hexadecimal word\n");
		instr->error |= ERR_SYNTAX;
		return;
	}

	decode_word(word, instr);
}
