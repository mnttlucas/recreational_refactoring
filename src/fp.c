#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "fp.h"
#include "mips_encoding.h"
#include "mips_registers.h"

const FpFormat fp_float = {32, 23};
const FpFormat fp_double = {64, 52};

static uint64_t fp_mantissa_mask(const FpFormat *format)
{
	return((UINT64_C(1) << format->mantissa_bits) - 1);
}

/* The exponent field, all ones for an infinity or a NaN */
static uint64_t fp_exponent_max(const FpFormat *format)
{
	return((UINT64_C(1) << (format->width - 1 - format->mantissa_bits)) - 1);
}

/* The most significant bit of the mantissa tells a quiet NaN from a signaling one */
static uint64_t fp_quiet_bit(const FpFormat *format)
{
	return(UINT64_C(1) << (format->mantissa_bits - 1));
}

static uint64_t fp_default_nan(const FpFormat *format)
{
	return((fp_exponent_max(format) << format->mantissa_bits) | fp_quiet_bit(format));
}

static uint64_t fp_read_bits(Cpu *cpu, int id, const FpFormat *format)
{
	return(format->width == 32 ? fpr_read_32(cpu, id) : fpr_read_64(cpu, id));
}

static void fp_write_bits(Cpu *cpu, int id, const FpFormat *format, uint64_t bits)
{
	if(format->width == 32) fpr_write_32(cpu, id, (uint32_t) bits);
	else fpr_write_64(cpu, id, bits);
}

/* The value of a register as a double : a float is converted exactly */
static double fp_read_value(Cpu *cpu, int id, const FpFormat *format)
{
	return(format->width == 32 ? (double) fpr_read_float(cpu, id) : fpr_read_double(cpu, id));
}

int fp_is_nan(const FpFormat *format, uint64_t bits)
{
	return(((bits >> format->mantissa_bits) & fp_exponent_max(format)) == fp_exponent_max(format) && (bits & fp_mantissa_mask(format)) != 0);
}

int fp_is_snan(const FpFormat *format, uint64_t bits)
{
	return(fp_is_nan(format, bits) && (bits & fp_quiet_bit(format)) == 0);
}

uint32_t fp_class(const FpFormat *format, uint64_t bits)
{
	const int negative = (int) ((bits >> (format->width - 1)) & 1);
	const uint64_t exponent = (bits >> format->mantissa_bits) & fp_exponent_max(format);
	const int mantissa_is_zero = (bits & fp_mantissa_mask(format)) == 0;

	if(exponent == fp_exponent_max(format))
	{
		if(mantissa_is_zero) return(negative ? FP_CLASS_NEG_INF : FP_CLASS_POS_INF);
		return(fp_is_snan(format, bits) ? FP_CLASS_SNAN : FP_CLASS_QNAN);
	}

	if(exponent == 0)
	{
		if(mantissa_is_zero) return(negative ? FP_CLASS_NEG_ZERO : FP_CLASS_POS_ZERO);
		return(negative ? FP_CLASS_NEG_SUBNORM : FP_CLASS_POS_SUBNORM);
	}

	return(negative ? FP_CLASS_NEG_NORMAL : FP_CLASS_POS_NORMAL);
}

static void fp_report_invalid(void)
{
	fprintf(stderr, "[!] Exception : Floating Point Invalid Operation\n");
}

static void fp_report_unimplemented(void)
{
	fprintf(stderr, "[!] Exception : Floating Point Unimplemented Operation\n");
}

/* The operands of the instruction that are selected, in the order fs, ft, fd */
static int fp_operand_registers(const Instruction *instr, unsigned operands, int registers[3])
{
	int count = 0;

	if(operands & FP_FS) registers[count++] = instr->fs;
	if(operands & FP_FT) registers[count++] = instr->ft;
	if(operands & FP_FD) registers[count++] = instr->fd;

	return(count);
}

static int fp_signal_invalid(Cpu *cpu, const Instruction *instr, const FpFormat *source, const FpFormat *destination, unsigned operands)
{
	int registers[3];
	const int count = fp_operand_registers(instr, operands, registers);

	for(int i = 0; i < count; i++)
	{
		if(!fp_is_snan(source, fp_read_bits(cpu, registers[i], source))) continue;

		fp_report_invalid();
		fp_write_bits(cpu, instr->fd, destination, fp_default_nan(destination));

		return(1);
	}

	return(0);
}

int fp_invalid(Cpu *cpu, const Instruction *instr, const FpFormat *format, unsigned operands)
{
	return(fp_signal_invalid(cpu, instr, format, format, operands));
}

int fp_invalid_convert(Cpu *cpu, const Instruction *instr, const FpFormat *source, const FpFormat *destination)
{
	return(fp_signal_invalid(cpu, instr, source, destination, FP_FS));
}

int fp_handle_nan(Cpu *cpu, const Instruction *instr, const FpFormat *format, unsigned operands)
{
	int registers[3];
	const int count = fp_operand_registers(instr, operands, registers);

	if(fp_invalid(cpu, instr, format, operands)) return(1);

	for(int i = 0; i < count; i++)
	{
		const uint64_t bits = fp_read_bits(cpu, registers[i], format);

		if(!fp_is_nan(format, bits)) continue;

		fp_write_bits(cpu, instr->fd, format, bits);

		return(1);
	}

	return(0);
}

float fp_result_float(float result, float a, float b)
{
	const uint32_t default_nan = (uint32_t) fp_default_nan(&fp_float);
	float nan;

	if(!isnan(result) || isnan(a) || isnan(b)) return(result);

	memcpy(&nan, &default_nan, sizeof(nan));

	return(nan);
}

double fp_result_double(double result, double a, double b)
{
	const uint64_t default_nan = fp_default_nan(&fp_double);
	double nan;

	if(!isnan(result) || isnan(a) || isnan(b)) return(result);

	memcpy(&nan, &default_nan, sizeof(nan));

	return(nan);
}

int fp_min_picks_fs(double a, double b)
{
	if(isnan(a)) return(0);
	if(isnan(b)) return(1);
	if(a == b) return(signbit(a) != 0);
	return(a < b);
}

int fp_max_picks_fs(double a, double b)
{
	if(isnan(a)) return(0);
	if(isnan(b)) return(1);
	if(a == b) return(signbit(a) == 0);
	return(a > b);
}

int fp_mina_picks_fs(double a, double b)
{
	if(isnan(a)) return(0);
	if(isnan(b)) return(1);
	if(fabs(a) == fabs(b)) return(fp_min_picks_fs(a, b));
	return(fabs(a) < fabs(b));
}

int fp_maxa_picks_fs(double a, double b)
{
	if(isnan(a)) return(0);
	if(isnan(b)) return(1);
	if(fabs(a) == fabs(b)) return(fp_max_picks_fs(a, b));
	return(fabs(a) > fabs(b));
}

/* CMP.cond.fmt : truth value of a condition, from the relation between the two operands.
   Exactly one of less, equal, unordered is set, or none of them when fs > ft. */
static int fp_compare_condition(uint32_t condition, int less, int equal, int unordered)
{
	switch(condition)
	{
		case FUNCT_FCMP_AF :
		case FUNCT_FCMP_SAF :
			return(0);
		case FUNCT_FCMP_UN :
		case FUNCT_FCMP_SUN :
			return(unordered);
		case FUNCT_FCMP_EQ :
		case FUNCT_FCMP_SEQ :
			return(equal);
		case FUNCT_FCMP_UEQ :
		case FUNCT_FCMP_SUEQ :
			return(unordered || equal);
		case FUNCT_FCMP_LT :
		case FUNCT_FCMP_SLT :
			return(less);
		case FUNCT_FCMP_ULT :
		case FUNCT_FCMP_SULT :
			return(unordered || less);
		case FUNCT_FCMP_LE :
		case FUNCT_FCMP_SLE :
			return(less || equal);
		case FUNCT_FCMP_ULE :
		case FUNCT_FCMP_SULE :
			return(unordered || less || equal);
		case FUNCT_FCMP_OR :
		case FUNCT_FCMP_SOR :
			return(!unordered);
		case FUNCT_FCMP_UNE :
		case FUNCT_FCMP_SUNE :
			return(!equal);
		case FUNCT_FCMP_NE :
		case FUNCT_FCMP_SNE :
			return(!unordered && !equal);
		default :
			/* Reserved condition (shaded entries of the CMP.condn.fmt table) : the result is not defined, 0 is written */
			fp_report_unimplemented();
			return(0);
	}
}

/* An Invalid Operation is reported for a signaling NaN operand, and for any NaN operand with the signaling
   conditions (SAF, SUN...). The result is still written : no trap, no FCSR. */
uint64_t fp_compare(Cpu *cpu, const Instruction *instr, const FpFormat *format)
{
	const uint64_t bits_a = fp_read_bits(cpu, instr->fs, format);
	const uint64_t bits_b = fp_read_bits(cpu, instr->ft, format);
	const double a = fp_read_value(cpu, instr->fs, format);
	const double b = fp_read_value(cpu, instr->ft, format);
	const uint32_t condition = instr->encoding & FUNCT_FCMP_MASK;
	const int unordered = isnan(a) || isnan(b);
	const uint64_t all_ones = UINT64_MAX >> (64 - format->width);

	if(fp_is_snan(format, bits_a) || fp_is_snan(format, bits_b) || (unordered && (condition & FUNCT_FCMP_SIGNAL))) fp_report_invalid();

	return(fp_compare_condition(condition, a < b, a == b, unordered) ? all_ones : 0);
}

/* Conversion to integer : a NaN gives 0, an infinity or a value out of range (after rounding) gives the nearest bound
   (FCSR.NAN2008 = 1, as required by R6). The Invalid Operation is not signaled, there is no FCSR.
   The FCSR rounding mode is not emulated : CVT uses round to nearest even (RN, the reset value),
   the others have their own fixed mode. */
static int32_t fp_to_int32(double x, double (*round_function)(double))
{
	double rounded;

	if(isnan(x)) return(0);

	rounded = round_function(x);
	if(rounded >= 2147483648.0) return(INT32_MAX);
	if(rounded < -2147483648.0) return(INT32_MIN);

	return((int32_t) rounded);
}

static int64_t fp_to_int64(double x, double (*round_function)(double))
{
	double rounded;

	if(isnan(x)) return(0);

	rounded = round_function(x);
	if(rounded >= 9223372036854775808.0) return(INT64_MAX);
	if(rounded < -9223372036854775808.0) return(INT64_MIN);

	return((int64_t) rounded);
}

void fp_to_integer(Cpu *cpu, const Instruction *instr, const FpFormat *source, int integer_width, double (*round_function)(double))
{
	const double x = fp_read_value(cpu, instr->fs, source);

	if(integer_width == 64) fpr_write_64(cpu, instr->fd, (uint64_t) fp_to_int64(x, round_function));
	else fpr_write_32(cpu, instr->fd, (uint32_t) fp_to_int32(x, round_function));
}
