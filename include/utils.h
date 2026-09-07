#ifndef	UTILS_H
#define	UTILS_H

#include "config.h"
#include "cpu.h"

void	bin_arr_to_hex_arr(int *tabBin, char *tabHex);
void	bin_twos_complement(int debut, int fin, int *tab);
void	bin_zero(int *tab);
void	cpu_dump(CPU *cpu, config *cfg);
void	long_to_bin_arr(int debut, int fin, long valeur, int *tab);
int		register_string_to_int(char *chaine);
void	remove_sign(char *chaine);

#endif	/* UTILS_H */