#ifndef PROIZVODI_H
#define PROIZVODI_H

typedef struct {
    int   id;
    char  ime[50];
    char  kategorija[30];
    float cijena;
} PROIZVOD;

void      izbornikProizvodi(const char* datoteka);
void      dodajProizvod(const char* datoteka);
PROIZVOD* ucitajProizvode(const char* datoteka, int* n);
int       spremiProizvode(const char* datoteka, PROIZVOD* p, int n);
void      ispisiProizvode(PROIZVOD* p, int n);
void      urediProizvod(const char* datoteka);
void      obrisiProizvod(const char* datoteka);
void      pretraziProizvode(PROIZVOD* p, int n);

#endif PROIZVODI_H 