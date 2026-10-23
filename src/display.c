#include <inttypes.h>
#include <stdio.h>

#include "display.h"
#include "fp.h"
#include "mips_memory.h"
#include "mips_registers.h"

typedef void (*dump_cell)(Cpu *cpu, const Config *cfg, int index);

/* Text of a float / double, from the raw bits. The host prints "nan" or "-nan" depending on the platform and
   does not tell the quiet NaNs from the signaling ones : both are spelled out here. */
static void format_nan_or_number(char *out, size_t size, const FpFormat *format, uint64_t bits, const char *number_format, double value)
{
	if(fp_is_nan(format, bits)) snprintf(out, size, "%s%s", ((bits >> (format->width - 1)) & 1) ? "-" : "", fp_is_snan(format, bits) ? "snan" : "qnan");
	else snprintf(out, size, number_format, value);
}

static void format_float(char *out, size_t size, uint32_t bits, float value)
{
	format_nan_or_number(out, size, &fp_float, bits, "%.9g", (double) value);
}

static void format_double(char *out, size_t size, uint64_t bits, double value)
{
	format_nan_or_number(out, size, &fp_double, bits, "%.17g", value);
}

static void print_gpr(Cpu *cpu, const Config *cfg, int index)
{
	uint32_t value = gpr_read_32(cpu, index);

	switch(cfg->gpr_format)
	{
		case GPR_HEX :
			printf("$%02d : 0x%08" PRIx32 "     ", index, value);
			break;
		case GPR_BOTH :
			printf("$%02d : %-11d 0x%08" PRIx32 "     ", index, (int32_t) value, value);
			break;
		default :
			printf("$%02d : %-10d ", index, (int32_t) value);
			break;
	}
}

static void print_fpr(Cpu *cpu, const Config *cfg, int index)
{
	char text_double[32], text_single[32];
	uint64_t bits = fpr_read_64(cpu, index);

	switch(cfg->fpr_format)
	{
		case FPR_FLOAT :
			format_float(text_single, sizeof(text_single), fpr_read_32(cpu, index), fpr_read_float(cpu, index));
			printf("$%02d : %-15s ", index, text_single);
			break;
		case FPR_DOUBLE :
			format_double(text_double, sizeof(text_double), bits, fpr_read_double(cpu, index));
			printf("$%02d : %-24s ", index, text_double);
			break;
		case FPR_ALL :
			format_double(text_double, sizeof(text_double), bits, fpr_read_double(cpu, index));
			format_float(text_single, sizeof(text_single), fpr_read_32(cpu, index), fpr_read_float(cpu, index));
			printf("$%02d : 0x%016" PRIx64 "  d=%-24s f=%s", index, bits, text_double, text_single);
			break;
		default :
			printf("$%02d : 0x%-16" PRIx64 " ", index, bits);
			break;
	}
}

static void print_memory_word(Cpu *cpu, const Config *cfg, int index)
{
	uint32_t address = (uint32_t) index * WORD_SIZE;

	(void) cfg;
	printf("@0000 %04" PRIx32 " : 0x%08" PRIx32 "       ", address, memory_read_32(cpu, address));
}

/* Number of cells per line, so that a line stays about 100 characters wide */
static int gpr_columns(const Config *cfg)
{
	return(cfg->gpr_format == GPR_BOTH ? 3 : DUMP_LINE_SIZE);
}

static int fpr_columns(const Config *cfg)
{
	switch(cfg->fpr_format)
	{
		case FPR_DOUBLE :
			return(3);
		case FPR_ALL :
			return(1);
		default :
			return(DUMP_LINE_SIZE);
	}
}

static void dump_grid(Cpu *cpu, const Config *cfg, int count, int columns, dump_cell print_cell)
{
	for(int i = 0; i < count; i++)
	{
		print_cell(cpu, cfg, i);
		if((i + 1) % columns == 0 || i == count - 1) printf("\n");
	}
}

void clear_output(void)
{
	printf("\033[H\033[2J\033[3J");
}

void cpu_dump(Cpu *cpu, const Config *cfg)
{
	int words_to_show = (MEMORY_SIZE / WORD_SIZE < DUMP_MEMORY_WORDS) ? MEMORY_SIZE / WORD_SIZE : DUMP_MEMORY_WORDS;

	printf("\n----------------------- GPR' status ----------------------\n");
	dump_grid(cpu, cfg, REGISTER_COUNT, gpr_columns(cfg), print_gpr);
	printf("\n------------------------------------ FPR' status -----------------------------------\n");
	dump_grid(cpu, cfg, REGISTER_COUNT, fpr_columns(cfg), print_fpr);
	printf("\n------------------------------------------------- Memory status -------------------------------------------------\n");
	dump_grid(cpu, cfg, words_to_show, DUMP_LINE_SIZE, print_memory_word);

	if(words_to_show < MEMORY_SIZE / WORD_SIZE) printf("\n\t\t\t\t\t[...] (%d other words not displayed)\n", MEMORY_SIZE / WORD_SIZE - words_to_show);
}

void log_instruction(Instruction *instr)
{
	if(!instr->exit) printf("%s", instr->to_string);
}

void log_usage(char *prog_name)
{
fprintf(stderr, "[!] Usage:\n \
- interactive mode -> \t\t%s [options]\n \
- step-by-step batch mode -> \t%s <prog.asm> -step [options]\n \
- batch mode -> \t\t%s <prog.asm> <out.hex> <out.regs> [options]\n\n \
Options (console display only, <out.regs> is not affected):\n \
-gpr=dec|hex|both\t\tgeneral purpose registers (default : dec)\n \
-fpr=hex|float|double|all\tfloating point registers (default : hex)\n \
\t\t\t\t(float : low 32 bits as a float)\n",
prog_name, prog_name, prog_name);
}

void wait_for_enter(void)
{
	int c;

	printf("\n[#] Press Enter to execute the next instruction");

	do
	{
		c = getchar();
	}
	while(c != '\n' && c != EOF);
}
