#ifndef KUPCI_H
#define KUPCI_H

/*  Struktura KUPAC (tocke 3, 4)*/
typedef struct {
    int  id;
    char ime[50];
    char prezime[50];
    char email[80];
    char telefon[20];
    char adresa[100];
} KUPAC;

/*  Enum tipovi izbornika (tocka 11)*/
typedef enum {
    KUPAC_DODAJ = 1,
    KUPAC_UCITAJ = 2,
    KUPAC_ISPISI = 3,
    KUPAC_UREDI = 4,
    KUPAC_OBRISI = 5,
    KUPAC_PRETRAZI = 6,
    KUPAC_SORTIRAJ = 8,
    KUPAC_POVRATAK = 7
} OpcijaKupci;

typedef enum {
    KUPAC_PRETRAGA_ID = 1,
    KUPAC_PRETRAGA_PREZIME = 2,
    KUPAC_PRETRAGA_EMAIL = 3,
    KUPAC_PRETRAGA_TELEFON = 4
} OpcijaKupacPretraga;


/*  Deklaracije funkcija*/

void    izbornikKupci(const char* datoteka);void    dodajKupca(const char* datoteka);
KUPAC* ucitajKupce(const char* datoteka, int* n);
int     spremiKupce(const char* datoteka, KUPAC* k, int n);
void    ispisiKupce(KUPAC* k, int n);
void    urediKupca(const char* datoteka);
void    obrisiKupca(const char* datoteka);
void    pretraziKupce(KUPAC* k, int n);
void    sortirajKupce(KUPAC* k, int n);
KUPAC* nadjiKupcaID(KUPAC* k, int n, int trazeniID);

#endif /* KUPCI_H */