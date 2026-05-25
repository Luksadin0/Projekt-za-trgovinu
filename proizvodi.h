#ifndef PROIZVODI_H
#define PROIZVODI_H

/* ------------------------------------------------------------------ */
/*  Struktura PROIZVOD (tocke 3, 4)                                    */
/* ------------------------------------------------------------------ */
typedef struct {
    int   id;
    char  ime[50];
    char  kategorija[30];
    float cijena;
} PROIZVOD;

/* ------------------------------------------------------------------ */
/*  Enum tipovi izbornika (tocka 11)                                   */
/* ------------------------------------------------------------------ */
typedef enum {
    PROIZVOD_DODAJ = 1,
    PROIZVOD_UCITAJ = 2,
    PROIZVOD_ISPISI = 3,
    PROIZVOD_UREDI = 4,
    PROIZVOD_OBRISI = 5,
    PROIZVOD_PRETRAZI = 6,
    PROIZVOD_POVRATAK = 7,
    PROIZVOD_SORTIRAJ = 8
} OpcijaProizvodi;

typedef enum {
    PROIZVOD_PRETRAGA_ID = 1,
    PROIZVOD_PRETRAGA_IME = 2,
    PROIZVOD_PRETRAGA_KATEGORIJA = 3,
    PROIZVOD_PRETRAGA_CIJENA = 4
} OpcijaProizvodPretraga;

/* ------------------------------------------------------------------ */
/*  Deklaracije funkcija                                               */
/* ------------------------------------------------------------------ */
void      izbornikProizvodi(const char* datoteka);
void      dodajProizvod(const char* datoteka);
PROIZVOD* ucitajProizvode(const char* datoteka, int* n);
int       spremiProizvode(const char* datoteka, PROIZVOD* p, int n);
void      ispisiProizvode(PROIZVOD* p, int n);
void      urediProizvod(const char* datoteka);
void      obrisiProizvod(const char* datoteka);
void      pretraziProizvode(PROIZVOD* p, int n);
void      sortirajProizvode(PROIZVOD* p, int n);

#endif /* PROIZVODI_H */