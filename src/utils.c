#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "registers.h"
#include "utils.h"

int register_string_to_int(char *reg)
{
	size_t i, longueur = strlen(reg);
	int idRegistre, r = 1;
	for(i = 0; i < longueur; i++)
	{
		if(isalpha(reg[i]))
		{
			r = 0;
		}
	}
	if(r){
		idRegistre = atoi(reg);
	}
	else{
		if(!strcmp(reg, "zero")){
			idRegistre = 0;
		}
		else if(!strcmp(reg, "at")){
			idRegistre = 1;
		}
		else if(!strcmp(reg, "gp")){
			idRegistre = 28;
		}
		else if(!strcmp(reg, "sp")){
			idRegistre = 29;
		}
		else if(!strcmp(reg, "fp")){
			idRegistre = 30;
		}
		else if(!strcmp(reg, "ra")){
			idRegistre = 31;
		}
		else if(reg[0] == 'v'){
			idRegistre = 2 + reg[1] - '0';
		}
		else if(reg[0] == 'a'){
			idRegistre = 4 + reg[1] - '0';
		}
		else if(reg[0] == 's'){
			idRegistre = 16 + reg[1] - '0';
		}
		else if(reg[0] == 'k'){
			idRegistre = 26 + reg[1] - '0';
		}
		else if(reg[0] == 't'){
			if(reg[1] - '0' <= 7){
				idRegistre = 8 + reg[1] - '0';
			}
			else{
				idRegistre = 24 + reg[1] - '0';
			}
		}
	}
	return(idRegistre);
}

void binary_zero(int *tab)
{
	int i;
	for(i = 0; i < 32; i++)
	{
		tab[i] = 0;
	}
}

void changeBin(int debut, int fin, long valeur, int *tab){
	int i;
	long bin = 1;
	for(i = debut; i < fin; i++){
		bin *= 2;
	}
	for(i = debut; i <= fin; i++){
		if(valeur >= bin){
			valeur -= bin;
			tab[i] = 1;
		}
		bin /= 2;
	}
}

void remove_sign(char *chaine)
{
	int i = 0;
	while(chaine[i + 1] != '\0')
	{
		chaine[i] = chaine[i + 1];
		i++;
	}
	chaine[i] = '\0';
}

void binary_twos_complement(int debut, int fin, int *tab)
{
	int i;
	for(i = debut; i <= fin; i++)
	{
		if(tab[i] == 0){
			tab[i] = 1;
		}
		else{
			tab[i] = 0;
		}
	}
	i = fin;
	while(tab[i] == 1){
		tab[i--] = 0;
	}
	tab[i] = 1;
}

void binary_to_hex(int *tabBin, char *tabHex){
	int i, j, m, v;
	for(i = 0; i < 8; i++){
		m = 8;
		v = 0;
		for(j = 0; j < 4; j++){
			v += m * tabBin[i * 4 + j];
			m /= 2;
		}
		if(v <= 9){
			tabHex[i] = (char) ('0' + v);
		}
		else{
			switch(v){
				case 10 : tabHex[i] = 'a'; break;
				case 11 : tabHex[i] = 'b'; break;
				case 12 : tabHex[i] = 'c'; break;
				case 13 : tabHex[i] = 'd'; break;
				case 14 : tabHex[i] = 'e'; break;
				case 15 : tabHex[i] = 'f'; break;
			}
		}
	}
	tabHex[8] = '\0';
}

void cpu_dump(CPU *cpu, config *cfg){
	int i, j, m;
	long v;
	char carHex[4] = {'0', '4', '8', 'C'}, space[2];
	printf("\033[2J\033[H");
	printf("-------------------- Registers' status -------------------\n");
	for(i = 0; i < 8; i++){
		for(j = 0; j < 4; j++){
			v = register_read(cpu, 4 * i + j);
			printf("$%s%d : %-10ld ", 4 * i + j <= 9 ? "0" : "", 4 * i + j, v);
		}
		printf("\n");
	}
	printf("                 HI  : %-10ld LO  : %-10ld\n", register_read(cpu, REG_HI), register_read(cpu, REG_LO));
	printf("\n");
	printf("------------------------------------ Memory status -----------------------------------\n");
	for(i = 0; i < 5; i++){
		for(j = 0; j < 4; j++){
			m = memory_read(cpu, 4 * i + j);
			printf("@0000 00%d%c : %-10d ", i, carHex[j], m);
		}
		printf("\n");
	}
	printf("\n");
	if(cfg->step)
		fgets(space, sizeof(space), stdin);
}