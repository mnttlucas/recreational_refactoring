#ifndef	UTILS_H
#define	UTILS_H

#include "config.h"
#include "cpu.h"

void	binary_zero(int *tab);
void	binary_to_hex(int *tabBin, char *tabHex);
void	binary_twos_complement(int debut, int fin, int *tab);
void	cpu_dump(CPU *cpu, config *cfg);
int		register_string_to_int(char *chaine);
void	remove_sign(char *chaine);
void	changeBin(int debut, int fin, long valeur, int *tab);

#endif	/* UTILS_H */