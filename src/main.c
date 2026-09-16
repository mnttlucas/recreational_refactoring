#include <string.h>

#include "config.h"
#include "cpu.h"
#include "mips_memory.h"
#include "modes.h"
#include "mips_registers.h"
#include "utils.h"

int main(int argc, char **argv)
{
	Config cfg = {0};
	CPU cpu;

	memory_init(&cpu);
	registers_init(&cpu);

	if(argc == 1) interactive_mode(&cpu, &cfg);
	else
	{
		cfg.verbose = 1;
		if(argc == 3 && !strcmp(argv[2], "-step"))
		{
			cfg.step = 1;
			batch_mode(&cpu, &cfg, argv[1], NULL, NULL);
		}
		else if(argc == 4) batch_mode(&cpu, &cfg, argv[1], argv[2], argv[3]);
		else log_usage(argv[0]);
	}
	return(0);
}