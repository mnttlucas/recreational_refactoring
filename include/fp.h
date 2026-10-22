#ifndef	FP_H
#define	FP_H

#include <stdint.h>

#include "cpu.h"
#include "instruction.h"

/* Layout of an IEEE 754 binary format : 1 sign bit, then the exponent, then the mantissa */
typedef struct
{
	int width;
	int mantissa_bits;
} FpFormat;

extern const FpFormat fp_float;
extern const FpFormat fp_double;

/* Source operands of an instruction, to select the ones that are checked */
#define FP_FS (1u << 0)
#define FP_FT (1u << 1)
#define FP_FD (1u << 2)

/* CLASS.fmt result : one bit is set (R6) */
#define FP_CLASS_SNAN        (1u << 0)
#define FP_CLASS_QNAN        (1u << 1)
#define FP_CLASS_NEG_INF     (1u << 2)
#define FP_CLASS_NEG_NORMAL  (1u << 3)
#define FP_CLASS_NEG_SUBNORM (1u << 4)
#define FP_CLASS_NEG_ZERO    (1u << 5)
#define FP_CLASS_POS_INF     (1u << 6)
#define FP_CLASS_POS_NORMAL  (1u << 7)
#define FP_CLASS_POS_SUBNORM (1u << 8)
#define FP_CLASS_POS_ZERO    (1u << 9)

/* Classification of raw register bits (the host types would quiet a signaling NaN) */
int fp_is_nan(const FpFormat *format, uint64_t bits);
int fp_is_snan(const FpFormat *format, uint64_t bits);
uint32_t fp_class(const FpFormat *format, uint64_t bits);

/* Invalid Operation on a signaling NaN in one of the selected operands : the default quiet NaN is written in fd
   and 1 is returned. The FCSR is not emulated, so there is no flag and no trap. */
int fp_invalid(Cpu *cpu, const Instruction *instr, const FpFormat *format, unsigned operands);

/* Same for a conversion between two formats (fs is the only operand, fd has the destination format) */
int fp_invalid_convert(Cpu *cpu, const Instruction *instr, const FpFormat *source, const FpFormat *destination);

/* fp_invalid(), then a quiet NaN found in the selected operands (in the order fs, ft, fd) is propagated as it is.
   Returns 1 when the result has been written. */
int fp_handle_nan(Cpu *cpu, const Instruction *instr, const FpFormat *format, unsigned operands);

/* An invalid operation without any NaN operand (inf - inf, sqrt(-1)...) produces the MIPS default quiet NaN,
   whereas the host produces its own NaN pattern. A NaN operand is propagated as it is, so the host result is kept. */
float fp_result_float(float result, float a, float b);
double fp_result_double(double result, double a, double b);

/* MIN / MAX / MINA / MAXA : 1 if the result is fs, 0 if it is ft */
int fp_min_picks_fs(double a, double b);
int fp_max_picks_fs(double a, double b);
int fp_mina_picks_fs(double a, double b);
int fp_maxa_picks_fs(double a, double b);

/* CMP.cond.fmt : all ones (true) or all zeros (false) in the width of the format */
uint64_t fp_compare(Cpu *cpu, const Instruction *instr, const FpFormat *format);

/* Conversion of fs to an integer of 32 or 64 bits in fd, rounded with round_function (floor, ceil, trunc, nearbyint) */
void fp_to_integer(Cpu *cpu, const Instruction *instr, const FpFormat *source, int integer_width, double (*round_function)(double));

#endif	/* FP_H */
