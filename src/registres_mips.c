#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registres_mips.h"

void initialiser_registres(){
	int i;
	for(i = 0; i < 35; i++){
		registres[i] = 0;
	}
}

int lire_registre(int identifiant){
	int valeur;
	valeur = registres[identifiant];
	return(valeur);
}

void ecrire_registre(int identifiant, int valeur){
	registres[identifiant] = valeur;
}