#ifndef __REGISTRES_MIPS__
#define __REGISTRES_MIPS__
typedef struct registre registre;

extern long registres[35];

void initialiser_registres();
int lire_registre(int identifiant);
void ecrire_registre(int identifiant, int valeur);

#endif