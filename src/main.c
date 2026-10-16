#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "cpu.h"
#include "display.h"
#include "mips_memory.h"
#include "modes.h"
#include "mips_registers.h"
#include "utils.h"

int main(int argc, char **argv)
{
	Config cfg = {0};
	Cpu cpu;
	int status = EXIT_SUCCESS;

	cpu_init(&cpu);

	if(argc == 1) interactive_mode(&cpu, &cfg);
	else
	{
		cfg.verbose = 1;
		if(argc == 3 && !strcmp(argv[2], "-step"))
		{
			cfg.step = 1;
			status = batch_mode(&cpu, &cfg, argv[1], NULL, NULL);
		}
		else if(argc == 4) status = batch_mode(&cpu, &cfg, argv[1], argv[2], argv[3]);
		else
		{
			status = EXIT_FAILURE;
			log_usage(argv[0]);
		}
	}
	return(status);
}
