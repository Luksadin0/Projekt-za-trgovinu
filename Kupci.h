#ifndef KUPCI_H
#define KUPCI_H

typedef struct {
    int  id;
    char ime[50];
    char prezime[50];
    char email[80];
    char telefon[20];
    char adresa[100];
} KUPAC;

void   izbornikKupci(const char* datoteka);
void   dodajKupca(const char* datoteka);
KUPAC* ucitajKupce(const char* datoteka, int* n);
int    spremiKupce(const char* datoteka, KUPAC* k, int n);
void   ispisiKupce(KUPAC* k, int n);
void   urediKupca(const char* datoteka);
void   obrisiKupca(const char* datoteka);
void   pretraziKupce(KUPAC* k, int n);

#endif 