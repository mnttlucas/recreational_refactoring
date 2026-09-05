#ifndef __MEMOIRE_MIPS__
#define __MEMOIRE_MIPS__

extern int memoire[20];

void initialiser_memoire();
int lire_memoire(int identifiant);
void ecrire_memoire(int identifiant, int valeur);

#endif