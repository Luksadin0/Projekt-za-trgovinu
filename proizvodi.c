#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "proizvodi.h"
#include "pomocnici.h"


static int sljedeciID(PROIZVOD* p, int n) {
    int maks = 0;
    for (int i = 0; i < n; i++)
        if (p[i].id > maks) maks = p[i].id;
    return maks + 1;
}


PROIZVOD* ucitajProizvode(const char* datoteka, int* n) {
    FILE* fp = fopen(datoteka, "rb");
    if (!fp) { *n = 0; return NULL; }

    if (fread(n, sizeof(int), 1, fp) != 1 || *n <= 0) {
        fclose(fp); *n = 0; return NULL;
    }

    PROIZVOD* p = malloc(*n * sizeof(PROIZVOD));
    if (!p) { fclose(fp); *n = 0; return NULL; }

    fread(p, sizeof(PROIZVOD), *n, fp);
    fclose(fp);
    return p;
}

int spremiProizvode(const char* datoteka, PROIZVOD* p, int n) {
    FILE* fp = fopen(datoteka, "wb");
    if (!fp) return 0;
    fwrite(&n, sizeof(int), 1, fp);
    if (n > 0) fwrite(p, sizeof(PROIZVOD), n, fp);
    fclose(fp);
    return 1;
}

void ispisiProizvode(PROIZVOD* p, int n) {
    if (!p || n == 0) {
        printf("Nema ucitanih proizvoda. Koristite opciju 'Ucitaj proizvode'.\n");
        return;
    }
    printf("\n%-5s %-30s %-20s %12s\n", "ID", "Ime", "Kategorija", "Cijena");
    printf("%-5s %-30s %-20s %12s\n",
        "-----", "------------------------------", "--------------------", "------------");
    for (int i = 0; i < n; i++)
        printf("%-5d %-30s %-20s %10.2f kn\n",
            p[i].id, p[i].ime, p[i].kategorija, p[i].cijena);
    printf("Ukupno: %d proizvod(a)\n", n);
}

void dodajProizvod(const char* datoteka) {
    int n = 0;
    PROIZVOD* p = ucitajProizvode(datoteka, &n);

    PROIZVOD novi;
    novi.id = sljedeciID(p, n);

    printf("Ime proizvoda   : "); citajLiniju(novi.ime, sizeof(novi.ime));
    printf("Kategorija      : "); citajLiniju(novi.kategorija, sizeof(novi.kategorija));

    printf("Cijena (kn)     : ");
    while (scanf("%f", &novi.cijena) != 1 || novi.cijena < 0) {
        printf("Nevazeci unos. Unesite pozitivan broj: ");
        ocistiBuffer();
    }
    ocistiBuffer();

    PROIZVOD* tmp = realloc(p, (n + 1) * sizeof(PROIZVOD));
    if (!tmp) { free(p); printf("Greska pri alokaciji memorije.\n"); return; }
    p = tmp;
    p[n++] = novi;

    if (spremiProizvode(datoteka, p, n))
        printf("Proizvod uspjesno dodan! (ID: %d)\n", novi.id);
    else
        printf("Greska pri spremanju datoteke.\n");
    free(p);
}

void urediProizvod(const char* datoteka) {
    int n = 0;
    PROIZVOD* p = ucitajProizvode(datoteka, &n);
    if (!p) { printf("Nema podataka za uredivanje.\n"); return; }

    ispisiProizvode(p, n);

    int id;
    printf("\nID proizvoda za uredivanje: ");
    scanf("%d", &id); ocistiBuffer();

    int idx = -1;
    for (int i = 0; i < n; i++)
        if (p[i].id == id) { idx = i; break; }

    if (idx == -1) {
        printf("Proizvod s ID-om %d nije pronaden.\n", id);
        free(p); return;
    }

    char buf[100];

    printf("Ime [%s]: ", p[idx].ime);
    citajLiniju(buf, sizeof(buf));
    if (buf[0] != '\0') strncpy(p[idx].ime, buf, sizeof(p[idx].ime) - 1);

    printf("Kategorija [%s]: ", p[idx].kategorija);
    citajLiniju(buf, sizeof(buf));
    if (buf[0] != '\0') strncpy(p[idx].kategorija, buf, sizeof(p[idx].kategorija) - 1);

    printf("Cijena [%.2f] (-1 za zadrzati): ", p[idx].cijena);
    float c;
    scanf("%f", &c); ocistiBuffer();
    if (c >= 0) p[idx].cijena = c;

    if (spremiProizvode(datoteka, p, n))
        printf("Proizvod ID %d uspjesno ureden.\n", id);
    else
        printf("Greska pri spremanju.\n");
    free(p);
}

void obrisiProizvod(const char* datoteka) {
    int n = 0;
    PROIZVOD* p = ucitajProizvode(datoteka, &n);
    if (!p) { printf("Nema podataka za brisanje.\n"); return; }

    ispisiProizvode(p, n);

    int id;
    printf("\nID proizvoda za brisanje: ");
    scanf("%d", &id); ocistiBuffer();

    int idx = -1;
    for (int i = 0; i < n; i++)
        if (p[i].id == id) { idx = i; break; }

    if (idx == -1) {
        printf("Proizvod s ID-om %d nije pronaden.\n", id);
        free(p); return;
    }

    char potvrda[10];
    printf("Brisanje proizvoda '%s' (%.2f kn). Potvrdite (da/ne): ",
        p[idx].ime, p[idx].cijena);
    citajLiniju(potvrda, sizeof(potvrda));

    if (strcmp(potvrda, "da") != 0) {
        printf("Brisanje odustano.\n");
        free(p); return;
    }

    /* Pomakni elemente ulijevo */
    for (int i = idx; i < n - 1; i++)
        p[i] = p[i + 1];
    n--;

    if (spremiProizvode(datoteka, p, n))
        printf("Proizvod uspjesno obrisan.\n");
    else
        printf("Greska pri spremanju.\n");
    free(p);
}

void pretraziProizvode(PROIZVOD* p, int n) {
    if (!p || n == 0) {
        printf("Nema ucitanih podataka. Koristite opciju 'Ucitaj proizvode'.\n");
        return;
    }

    int pron, opcija;
    printf("\nPretraga po:\n");
    printf("  1. ID-u\n  2. Imenu\n  3. Kategoriji\n  4. Rasponu cijena\n");
    printf("Izbor: ");
    scanf("%d", &opcija); ocistiBuffer();

    pron = 0;
    printf("\n%-5s %-30s %-20s %12s\n", "ID", "Ime", "Kategorija", "Cijena");
    printf("%-5s %-30s %-20s %12s\n",
        "-----", "------------------------------", "--------------------", "------------");

    switch (opcija) {
    case 1: {
        int id;
        printf("ID: "); scanf("%d", &id); ocistiBuffer();
        for (int i = 0; i < n; i++)
            if (p[i].id == id) {
                printf("%-5d %-30s %-20s %10.2f kn\n",
                    p[i].id, p[i].ime, p[i].kategorija, p[i].cijena);
                pron = 1; break;
            }
        break;
    }
    case 2: {
        char upit[50];
        printf("Dio imena: "); citajLiniju(upit, sizeof(upit));
        for (int i = 0; i < n; i++)
            if (strstr(p[i].ime, upit)) {
                printf("%-5d %-30s %-20s %10.2f kn\n",
                    p[i].id, p[i].ime, p[i].kategorija, p[i].cijena);
                pron = 1;
            }
        break;
    }
    case 3: {
        char upit[30];
        printf("Kategorija: "); citajLiniju(upit, sizeof(upit));
        for (int i = 0; i < n; i++)
            if (strstr(p[i].kategorija, upit)) {
                printf("%-5d %-30s %-20s %10.2f kn\n",
                    p[i].id, p[i].ime, p[i].kategorija, p[i].cijena);
                pron = 1;
            }
        break;
    }
    case 4: {
        float mn, mx;
        printf("Min cijena: "); scanf("%f", &mn); ocistiBuffer();
        printf("Max cijena: "); scanf("%f", &mx); ocistiBuffer();
        for (int i = 0; i < n; i++)
            if (p[i].cijena >= mn && p[i].cijena <= mx) {
                printf("%-5d %-30s %-20s %10.2f kn\n",
                    p[i].id, p[i].ime, p[i].kategorija, p[i].cijena);
                pron = 1;
            }
        break;
    }
    default:
        printf("Nevazeci izbor.\n"); return;
    }

    if (!pron) printf("Nista pronadeno.\n");
}

void izbornikProizvodi(const char* datoteka) {
    int       izbor;
    PROIZVOD* p = NULL;
    int       n = 0;

    while (1) {
        printf("\n--- UPRAVLJANJE PROIZVODIMA ---\n");
        printf("  1. Dodaj novi proizvod\n");
        printf("  2. Ucitaj proizvode iz datoteke\n");
        printf("  3. Ispisi sve proizvode\n");
        printf("  4. Uredi proizvod\n");
        printf("  5. Obrisi proizvod\n");
        printf("  6. Pretrazi proizvode\n");
        printf("  7. Povratak\n");
        printf("Izbor: ");
        scanf("%d", &izbor); ocistiBuffer();

        switch (izbor) {
        case 1: dodajProizvod(datoteka); break;
        case 2:
            if (p) free(p);
            p = ucitajProizvode(datoteka, &n);
            printf(p ? "Ucitano %d proizvoda.\n" : "Datoteka prazna ili ne postoji.\n", n);
            break;
        case 3: ispisiProizvode(p, n); break;
        case 4:
            urediProizvod(datoteka);
            if (p) { free(p); p = ucitajProizvode(datoteka, &n); }
            break;
        case 5:
            obrisiProizvod(datoteka);
            if (p) { free(p); p = ucitajProizvode(datoteka, &n); }
            break;
        case 6: pretraziProizvode(p, n); break;
        case 7:
            if (p) { free(p); p = NULL; n = 0; }
            return;
        default:
            printf("Nevazeci izbor.\n");
        }
    }
}