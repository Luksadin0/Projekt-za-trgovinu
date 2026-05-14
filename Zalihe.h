#ifndef ZALIHE_H
#define ZALIHE_H

typedef struct {
    int  proizvod_id;
    char naziv[50];
    int  kolicina;
    int  min_kolicina;   /* Upozorenje kad padne ispod ove razine */
    char lokacija[50];   /* Npr. "Skladiste A, polica 3" */
} ZALIHA;

void    izbornikZalihe(const char* datoteka);
void    dodajZalihu(const char* datoteka);
ZALIHA* ucitajZalihe(const char* datoteka, int* n);
int     spremiZalihe(const char* datoteka, ZALIHA* z, int n);
void    ispisiZalihe(ZALIHA* z, int n);
void    urediZalihu(const char* datoteka);
void    obrisiZalihu(const char* datoteka);
void    prikaziNiskuZalihu(ZALIHA* z, int n);

#endif 