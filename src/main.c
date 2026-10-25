#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "cpu.h"
#include "display.h"
#include "mips_memory.h"
#include "modes.h"
#include "mips_registers.h"
#include "utils.h"

#define MAX_FILES 3
#define HEX_FILES (MAX_FILES - 1)	/* -hex : no .hex to write, the input already is one */

typedef struct
{
	const char *name;
	int value;
} OptionValue;

/* Values accepted by -gpr= and -fpr=, the tables end with a NULL name */
static const OptionValue gpr_formats[] =
{
	{"dec", GPR_DEC},
	{"hex", GPR_HEX},
	{"both", GPR_BOTH},
	{NULL, 0}
};

static const OptionValue fpr_formats[] =
{
	{"hex", FPR_HEX},
	{"float", FPR_FLOAT},
	{"double", FPR_DOUBLE},
	{"all", FPR_ALL},
	{NULL, 0}
};

static int lookup_value(const OptionValue *table, const char *text, int *value)
{
	for(; table->name; table++)
	{
		if(!strcmp(table->name, text))
		{
			*value = table->value;
			return(1);
		}
	}

	return(0);
}

/* Handles one "-option" argument, returns 0 if it is unknown or if its value is invalid */
static int parse_option(const char *arg, Config *cfg)
{
	int value;

	if(!strcmp(arg, "-step")) cfg->step = 1;
	else if(!strcmp(arg, "-hex")) cfg->hex = 1;
	else if(!strncmp(arg, "-gpr=", 5) && lookup_value(gpr_formats, arg + 5, &value)) cfg->gpr_format = (GprFormat) value;
	else if(!strncmp(arg, "-fpr=", 5) && lookup_value(fpr_formats, arg + 5, &value)) cfg->fpr_format = (FprFormat) value;
	else
	{
		fprintf(stderr, "[!] Unknown option or invalid value : '%s'\n", arg);
		return(0);
	}

	return(1);
}

int main(int argc, char **argv)
{
	char *files[MAX_FILES];
	Config cfg = {0};
	Cpu cpu;
	int file_count = 0, status = EXIT_SUCCESS;

	/* Options can be anywhere, everything else is a file name */
	for(int i = 1; i < argc; i++)
	{
		if(argv[i][0] == '-')
		{
			if(!parse_option(argv[i], &cfg))
			{
				log_usage(argv[0]);
				return(EXIT_FAILURE);
			}
		}
		else if(file_count < MAX_FILES) files[file_count++] = argv[i];
		else
		{
			log_usage(argv[0]);
			return(EXIT_FAILURE);
		}
	}

	cpu_init(&cpu);
	cfg.verbose = file_count > 0;

	if(file_count == 0 && !cfg.step) interactive_mode(&cpu, &cfg);
	else if(cfg.step && file_count == 1) status = batch_mode(&cpu, &cfg, files[0], NULL, NULL);
	else if(!cfg.step && !cfg.hex && file_count == MAX_FILES) status = batch_mode(&cpu, &cfg, files[0], files[1], files[2]);
	else if(!cfg.step && cfg.hex && file_count == HEX_FILES) status = batch_mode(&cpu, &cfg, files[0], NULL, files[1]);
	else
	{
		status = EXIT_FAILURE;
		log_usage(argv[0]);
	}

	return(status);
}
