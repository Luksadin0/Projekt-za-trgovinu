#ifndef ZALIHE_H
#define ZALIHE_H

/* ------------------------------------------------------------------ */
/*  Struktura ZALIHA (tocke 3, 4)                                      */
/* ------------------------------------------------------------------ */
typedef struct {
    int  proizvod_id;
    char naziv[50];
    int  kolicina;
    int  min_kolicina;
    char lokacija[50];
} ZALIHA;

/* ------------------------------------------------------------------ */
/*  Enum tipovi izbornika (tocka 11)                                   */
/* ------------------------------------------------------------------ */
typedef enum {
    ZALIHA_DODAJ = 1,
    ZALIHA_UCITAJ = 2,
    ZALIHA_ISPISI = 3,
    ZALIHA_UREDI = 4,
    ZALIHA_OBRISI = 5,
    ZALIHA_NISKA = 6,
    ZALIHA_POVRATAK = 7,
    ZALIHA_SORTIRAJ = 8
} OpcijaZalihe;

/* ------------------------------------------------------------------ */
/*  Deklaracije funkcija                                               */
/* ------------------------------------------------------------------ */
void    izbornikZalihe(const char* datoteka);
void    dodajZalihu(const char* datoteka);
ZALIHA* ucitajZalihe(const char* datoteka, int* n);
int     spremiZalihe(const char* datoteka, ZALIHA* z, int n);
void    ispisiZalihe(ZALIHA* z, int n);
void    urediZalihu(const char* datoteka);
void    obrisiZalihu(const char* datoteka);
void    prikaziNiskuZalihu(ZALIHA* z, int n);
void    sortirajZalihe(ZALIHA* z, int n);

#endif /* ZALIHE_H */