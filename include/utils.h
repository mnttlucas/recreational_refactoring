#ifndef	UTILS_H
#define	UTILS_H

#include <stdio.h>

#include "config.h"
#include "cpu.h"
#include "instruction.h"

#define DUMP_MEMORY_WORDS 40

#define ARR_SIZE(arr) \
	(sizeof(arr) / sizeof((arr)[0]))

void	bin_arr_to_hex_arr(uint8_t *bin_arr, char *hex_arr);
void	bin_twos_complement(int debut, int fin, uint8_t *tab);
void	clear_output();
void	cpu_dump(CPU *cpu, Config *cfg);
void	increment_pc(CPU *cpu);
int 	is_negative(char *str);
void	log_instruction(instruction *instr);
void	log_usage(char *prog_name);
void	long_to_bin_arr(int start, int end, long value, uint8_t *arr);
char	*read_full_line(FILE *in);
uint8_t	register_string_to_int(char *str);
void	remove_sign(char *str);
void	wait_for_enter();

#endif	/* UTILS_H */