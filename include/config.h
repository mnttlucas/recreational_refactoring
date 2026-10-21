#ifndef	CONFIG_H
#define	CONFIG_H

/* Display of the registers (console only, the out.regs file is not affected). The zero values are the historical display. */
typedef enum
{
	GPR_DEC,		/* signed decimal */
	GPR_HEX,		/* 32 bits hexadecimal */
	GPR_BOTH		/* both of them */
} GprFormat;

typedef enum
{
	FPR_HEX,		/* 64 bits hexadecimal */
	FPR_FLOAT,		/* low 32 bits as a float */
	FPR_DOUBLE,		/* 64 bits as a double */
	FPR_ALL			/* hexadecimal, double and float (low 32 bits), one register per line */
} FprFormat;

typedef struct
{
	int step;
	int verbose;
	GprFormat gpr_format;
	FprFormat fpr_format;
} Config;

#endif	/* CONFIG_H */
