#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memoire_mips.h"

void initialiser_memoire(){
	int i;
	for(i = 0; i < 35; i++){
		memoire[i] = 0;
	}
}

int lire_memoire(int identifiant){
	int valeur;
	valeur = memoire[identifiant];
	return(valeur);
}

void ecrire_memoire(int identifiant, int valeur){
	memoire[identifiant] = valeur;
}