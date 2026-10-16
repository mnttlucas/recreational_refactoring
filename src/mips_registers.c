#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cpu.h"
#include "mips_registers.h"
#include "utils.h"

typedef struct
{
	const char *name;
	uint8_t index;
} register_alias;

void registers_init(Cpu *cpu)
{
	for(int i = 0; i < REGISTER_COUNT; i++)
	{
		cpu->FPR[i] = 0;
		cpu->GPR[i] = 0;
	}
}

uint32_t fpr_read_32(Cpu *cpu, int id)
{
	uint32_t value = 0;

	if(id >= 0 && id < REGISTER_COUNT) value = (uint32_t) cpu->FPR[id];
	else fprintf(stderr, "[!] fpr_read_32() : invalid register %d\n", id);

	return(value);
}

uint64_t fpr_read_64(Cpu *cpu, int id)
{
	uint64_t value = 0;

	if(id >= 0 && id < REGISTER_COUNT) value = cpu->FPR[id];
	else fprintf(stderr, "[!] fpr_read_64() : invalid register %d\n", id);

	return(value);
}

void fpr_write_32(Cpu *cpu, int id, uint32_t value)
{
	if(id >= 0 && id < REGISTER_COUNT) cpu->FPR[id] = (uint64_t) value;
	else fprintf(stderr, "[!] fpr_write_32() : invalid register %d\n", id);
}

void fpr_write_64(Cpu *cpu, int id, uint64_t value)
{
	if(id >= 0 && id < REGISTER_COUNT) cpu->FPR[id] = value;
	else fprintf(stderr, "[!] fpr_write_64() : invalid register %d\n", id);
}

uint32_t gpr_read_32(Cpu *cpu, int id)
{
	uint32_t value = 0;

	if(id >= 0 && id < REGISTER_COUNT) value = cpu->GPR[id];
	else fprintf(stderr, "[!] gpr_read_32() : invalid register %d\n", id);
	
	return(value);
}

void gpr_write_32(Cpu *cpu, int id, uint32_t value)
{
	if(id > 0 && id < REGISTER_COUNT) cpu->GPR[id] = value;
	else if(id == 0);
	else fprintf(stderr, "[!] gpr_write_32() : invalid register %d\n", id);
}

/* Register names (without the leading '$'), see MIPS32 o32 ABI */
static const register_alias register_aliases[] = {
	{"zero", REG_ZERO}, {"at", REG_AT},
	{"v0", REG_V0}, {"v1", REG_V1},
	{"a0", REG_A0}, {"a1", REG_A1}, {"a2", REG_A2}, {"a3", REG_A3},
	{"t0", REG_T0}, {"t1", REG_T1}, {"t2", REG_T2}, {"t3", REG_T3},
	{"t4", REG_T4}, {"t5", REG_T5}, {"t6", REG_T6}, {"t7", REG_T7},
	{"s0", REG_S0}, {"s1", REG_S1}, {"s2", REG_S2}, {"s3", REG_S3},
	{"s4", REG_S4}, {"s5", REG_S5}, {"s6", REG_S6}, {"s7", REG_S7},
	{"t8", REG_T8}, {"t9", REG_T9},
	{"k0", REG_K0}, {"k1", REG_K1},
	{"gp", REG_GP}, {"sp", REG_SP}, {"fp", REG_FP}, {"ra", REG_RA}};

/* Parses a register number made of one or two digits ("5", "05", "31").
   Returns INVALID_REGISTER if there is anything else or if it is out of range. */
static uint8_t parse_register_number(const char *str)
{
	int value = 0;
	size_t length = strlen(str);

	if(length == 0 || length > 2) return(INVALID_REGISTER);

	for(size_t i = 0; i < length; i++)
	{
		if(str[i] < '0' || str[i] > '9') return(INVALID_REGISTER);
		value = value * 10 + (str[i] - '0');
	}

	return(value < REGISTER_COUNT ? (uint8_t) value : INVALID_REGISTER);
}

/* Accepts "$5", "$t0", "$T0", "$f12" without the '$' : number, alias or FPR name.
   Returns INVALID_REGISTER (and prints the reason) when the name is not a register. */
uint8_t register_string_to_int(const char *reg)
{
	uint8_t reg_int = parse_register_number(reg);

	if(reg_int == INVALID_REGISTER && (reg[0] == 'f' || reg[0] == 'F')) reg_int = parse_register_number(reg + 1);

	for(size_t i = 0; reg_int == INVALID_REGISTER && i < ARR_SIZE(register_aliases); i++) if(!mystrcasecmp(reg, register_aliases[i].name)) reg_int = register_aliases[i].index;

	if(reg_int == INVALID_REGISTER) fprintf(stderr, "[!] Unknown register : '%s'\n", reg);

	return(reg_int);
}
