#ifndef	CONFIG_H
#define	CONFIG_H

/* Display of the registers (console only, the out.regs file is not affected). The zero values are the historical display. */

typedef enum
{
	FPR_ALL,		/* hexadecimal, double and float (low 32 bits), one register per line */
	FPR_DOUBLE,		/* 64 bits as a double */
	FPR_FLOAT,		/* low 32 bits as a float */
	FPR_HEX			/* 64 bits hexadecimal */
} FprFormat;

typedef enum
{
	GPR_BOTH,		/* both of them */
	GPR_DEC,		/* signed decimal */
	GPR_HEX			/* 32 bits hexadecimal */
} GprFormat;

typedef struct
{
	FprFormat fpr_format;
	GprFormat gpr_format;
	int step;
	int verbose;
} Config;

#endif	/* CONFIG_H */
