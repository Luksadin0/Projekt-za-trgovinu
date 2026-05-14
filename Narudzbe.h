#ifndef NARUDZBE_H
#define NARUDZBE_H

typedef struct {
    int   id;
    int   kupac_id;
    int   proizvod_id;
    int   kolicina;
    float ukupna_cijena;
    char  datum[20];
    char  status[20];   /* Na cekanju, Isporuceno, Otkazano */
} NARUDZBA;

void      izbornikNarudzbe(const char* dat_nar,
    const char* dat_kupci,
    const char* dat_proizvodi);
void      dodajNarudzbu(const char* dat_nar,
    const char* dat_kupci,
    const char* dat_proizvodi);
NARUDZBA* ucitajNarudzbe(const char* datoteka, int* n);
int        spremiNarudzbe(const char* datoteka, NARUDZBA* nar, int n);
void      ispisiNarudzbe(NARUDZBA* nar, int n);
void      urediNarudzbu(const char* datoteka);
void      obrisiNarudzbu(const char* datoteka);
void      pretraziNarudzbe(NARUDZBA* nar, int n);

#endif 