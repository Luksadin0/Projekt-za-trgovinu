#ifndef NARUDZBE_H
#define NARUDZBE_H

/* ------------------------------------------------------------------ */
/*  Struktura NARUDZBA (tocke 3, 4)                                    */
/* ------------------------------------------------------------------ */
typedef struct {
    int   id;
    int   kupac_id;
    int   proizvod_id;
    int   kolicina;
    float ukupna_cijena;
    char  datum[20];
    char  status[20];   /* Na cekanju | Isporuceno | Otkazano */
} NARUDZBA;

/* ------------------------------------------------------------------ */
/*  Enum tipovi izbornika (tocka 11)                                   */
/* ------------------------------------------------------------------ */
typedef enum {
    NARUDZBA_DODAJ = 1,
    NARUDZBA_UCITAJ = 2,
    NARUDZBA_ISPISI = 3,
    NARUDZBA_UREDI = 4,
    NARUDZBA_OBRISI = 5,
    NARUDZBA_PRETRAZI = 6,
    NARUDZBA_POVRATAK = 7,
    NARUDZBA_SORTIRAJ = 8
} OpcijaNarudzbe;

typedef enum {
    NARUDZBA_PRETRAGA_ID = 1,
    NARUDZBA_PRETRAGA_KUPAC_ID = 2,
    NARUDZBA_PRETRAGA_PROIZVOD_ID = 3,
    NARUDZBA_PRETRAGA_STATUS = 4
} OpcijaNarudzbaPretraga;

typedef enum {
    STATUS_NA_CEKANJU = 1,
    STATUS_ISPORUCENO = 2,
    STATUS_OTKAZANO = 3,
    STATUS_ZADRZATI = 0
} OpcijaStatus;

/* ------------------------------------------------------------------ */
/*  Deklaracije funkcija                                               */
/* ------------------------------------------------------------------ */
void      izbornikNarudzbe(const char* dat_nar,
    const char* dat_kupci,
    const char* dat_proizvodi);
void      dodajNarudzbu(const char* dat_nar,
    const char* dat_kupci,
    const char* dat_proizvodi);
NARUDZBA* ucitajNarudzbe(const char* datoteka, int* n);
int       spremiNarudzbe(const char* datoteka, NARUDZBA* nar, int n);
void      ispisiNarudzbe(NARUDZBA* nar, int n);
void      urediNarudzbu(const char* datoteka);
void      obrisiNarudzbu(const char* datoteka);
void      pretraziNarudzbe(NARUDZBA* nar, int n);
void      sortirajNarudzbe(NARUDZBA* nar, int n);

#endif /* NARUDZBE_H */