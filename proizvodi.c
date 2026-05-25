#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "proizvodi.h"
#include "pomocnici.h"

/* ------------------------------------------------------------------ */
/*  Static pomocne funkcije (tocka 6)                                  */
/* ------------------------------------------------------------------ */

static int sljedeciID(PROIZVOD* p, int n) {
    int maks = 0;
    for (int i = 0; i < n; i++)
        if (p[i].id > maks) maks = p[i].id;
    return maks + 1;
}

/* ------------------------------------------------------------------ */
/*  Komparatori za qsort i bsearch (tocke 23, 24, 26)                  */
/* ------------------------------------------------------------------ */

/* Pokazivac na funkciju – sortiranje po cijeni */
static int usporediCijenu(const void* a, const void* b) {
    float razlika = ((const PROIZVOD*)a)->cijena -
        ((const PROIZVOD*)b)->cijena;
    return (razlika > 0.0f) - (razlika < 0.0f);
}

/* Komparator za bsearch po ID-u */
static int usporediProizvodID(const void* kljuc, const void* elem) {
    int id = *(const int*)kljuc;
    return id - ((const PROIZVOD*)elem)->id;
}

/* ------------------------------------------------------------------ */
/*  Rekurzivni binarni search po ID-u (tocka 25)                       */
/* ------------------------------------------------------------------ */
static int binSearchProizvodRek(PROIZVOD* p, int lijevo, int desno,
    int trazeniID) {
    if (lijevo > desno) return -1;
    int sredina = lijevo + (desno - lijevo) / 2;
    if (p[sredina].id == trazeniID) return sredina;
    if (p[sredina].id < trazeniID)
        return binSearchProizvodRek(p, sredina + 1, desno, trazeniID);
    return binSearchProizvodRek(p, lijevo, sredina - 1, trazeniID);
}

/* ------------------------------------------------------------------ */
/*  CRUD funkcije                                                       */
/* ------------------------------------------------------------------ */

PROIZVOD* ucitajProizvode(const char* datoteka, int* n) {
    if (!datoteka || !n) return NULL;

    FILE* fp = fopen(datoteka, "rb");
    if (!fp) { *n = 0; return NULL; }

    rewind(fp);   /* tocka 20 */

    if (fread(n, sizeof(int), 1, fp) != 1 || *n <= 0) {
        if (ferror(fp)) ispisiGresku("fread (count)");  /* tocka 22 */
        fclose(fp); *n = 0; return NULL;
    }

    long pos = ftell(fp);   /* tocka 20 */
    if (pos < 0) { ispisiGresku("ftell"); fclose(fp); *n = 0; return NULL; }

    PROIZVOD* p = malloc((size_t)(*n) * sizeof(PROIZVOD));  /* tocka 17 */
    if (!p) { ispisiGresku("malloc"); fclose(fp); *n = 0; return NULL; }

    if ((int)fread(p, sizeof(PROIZVOD), (size_t)(*n), fp) != *n) {
        if (ferror(fp)) ispisiGresku("fread (data)");
        free(p); p = NULL; *n = 0; fclose(fp); return NULL;  /* tocka 18 */
    }

    fclose(fp);
    return p;
}

int spremiProizvode(const char* datoteka, PROIZVOD* p, int n) {
    if (!datoteka) return 0;
    return atomicnoSpremi(datoteka, p, n, sizeof(PROIZVOD));  /* tocka 21 */
}

void ispisiProizvode(PROIZVOD* p, int n) {
    if (!p || n <= 0) {   /* tocka 14 */
        printf("Nema ucitanih proizvoda. Koristite opciju 'Ucitaj proizvode'.\n");
        return;
    }
    printf("\n%-5s %-30s %-20s %12s\n", "ID", "Ime", "Kategorija", "Cijena");
    printf("%-5s %-30s %-20s %12s\n",
        "-----", "------------------------------",
        "--------------------", "------------");
    for (int i = 0; i < n; i++)
        printf("%-5d %-30s %-20s %10.2f EUR\n",
            p[i].id, p[i].ime, p[i].kategorija, p[i].cijena);
    printf("Ukupno: %d proizvod(a)\n", n);
}

void dodajProizvod(const char* datoteka) {
    if (!datoteka) return;

    int       n = 0;
    PROIZVOD* p = ucitajProizvode(datoteka, &n);

    PROIZVOD novi;
    memset(&novi, 0, sizeof(PROIZVOD));
    novi.id = sljedeciID(p, n);

    printf("Ime proizvoda   : "); citajLiniju(novi.ime, sizeof(novi.ime));
    printf("Kategorija      : "); citajLiniju(novi.kategorija, sizeof(novi.kategorija));

    printf("Cijena (EUR)    : ");
    while (scanf("%f", &novi.cijena) != 1 || novi.cijena < 0.0f) {
        printf("Nevazeci unos. Unesite pozitivan broj: ");
        ocistiBuffer();
    }
    ocistiBuffer();

    PROIZVOD* tmp = realloc(p, (size_t)(n + 1) * sizeof(PROIZVOD));  /* tocka 17 */
    if (!tmp) {
        ispisiGresku("realloc");
        free(p); p = NULL; return;   /* tocka 18 */
    }
    p = tmp;
    p[n++] = novi;

    if (spremiProizvode(datoteka, p, n))
        printf("Proizvod uspjesno dodan! (ID: %d)\n", novi.id);
    else
        printf("Greska pri spremanju datoteke.\n");

    free(p); p = NULL;
}

void urediProizvod(const char* datoteka) {
    if (!datoteka) return;

    int       n = 0;
    PROIZVOD* p = ucitajProizvode(datoteka, &n);
    if (!p) { printf("Nema podataka za uredivanje.\n"); return; }

    ispisiProizvode(p, n);

    int id;
    printf("\nID proizvoda za uredivanje: ");
    scanf("%d", &id); ocistiBuffer();

    /* tocka 25 – rekurzivni binary search */
    int idx = binSearchProizvodRek(p, 0, n - 1, id);
    if (idx == -1) {
        printf("Proizvod s ID-om %d nije pronaden.\n", id);
        free(p); p = NULL; return;
    }

    char buf[100];

    printf("Ime [%s]: ", p[idx].ime);
    citajLiniju(buf, sizeof(buf));
    if (buf[0] != '\0') sigurnoKopiraj(p[idx].ime, buf, sizeof(p[idx].ime));

    printf("Kategorija [%s]: ", p[idx].kategorija);
    citajLiniju(buf, sizeof(buf));
    if (buf[0] != '\0') sigurnoKopiraj(p[idx].kategorija, buf, sizeof(p[idx].kategorija));

    printf("Cijena [%.2f] (-1 za zadrzati): ", p[idx].cijena);
    float c;
    scanf("%f", &c); ocistiBuffer();
    if (c >= 0.0f) p[idx].cijena = c;

    if (spremiProizvode(datoteka, p, n))
        printf("Proizvod ID %d uspjesno ureden.\n", id);
    else
        printf("Greska pri spremanju.\n");

    free(p); p = NULL;
}

void obrisiProizvod(const char* datoteka) {
    if (!datoteka) return;

    int       n = 0;
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
        free(p); p = NULL; return;
    }

    char potvrda[10];
    printf("Brisanje proizvoda '%s' (%.2f EUR). Potvrdite (da/ne): ",
        p[idx].ime, p[idx].cijena);
    citajLiniju(potvrda, sizeof(potvrda));

    if (strcmp(potvrda, "da") != 0) {
        printf("Brisanje odustano.\n");
        free(p); p = NULL; return;
    }

    for (int i = idx; i < n - 1; i++)
        p[i] = p[i + 1];
    n--;
    memset(&p[n], 0, sizeof(PROIZVOD));   /* tocka 18 – anuliranje */

    if (spremiProizvode(datoteka, p, n))
        printf("Proizvod uspjesno obrisan.\n");
    else
        printf("Greska pri spremanju.\n");

    free(p); p = NULL;
}

/* ------------------------------------------------------------------ */
/*  Sortiranje i pretraga (tocke 23, 24, 26)                           */
/* ------------------------------------------------------------------ */

void sortirajProizvode(PROIZVOD* p, int n) {
    if (!p || n <= 1) return;
    /* tocka 23 – qsort, tocka 26 – pokazivac na funkciju usporediCijenu */
    qsort(p, (size_t)n, sizeof(PROIZVOD), usporediCijenu);
}

void pretraziProizvode(PROIZVOD* p, int n) {
    if (!p || n <= 0) {
        printf("Nema ucitanih podataka. Koristite opciju 'Ucitaj proizvode'.\n");
        return;
    }

    int opcija, pron = 0;
    printf("\nPretraga po:\n");
    printf("  1. ID-u\n  2. Imenu\n  3. Kategoriji\n  4. Rasponu cijena\n");
    printf("Izbor: ");
    scanf("%d", &opcija); ocistiBuffer();

    printf("\n%-5s %-30s %-20s %12s\n", "ID", "Ime", "Kategorija", "Cijena");
    printf("%-5s %-30s %-20s %12s\n",
        "-----", "------------------------------",
        "--------------------", "------------");

    switch ((OpcijaProizvodPretraga)opcija) {
    case PROIZVOD_PRETRAGA_ID: {
        int id;
        printf("ID: "); scanf("%d", &id); ocistiBuffer();
        /* tocka 24 – bsearch */
        PROIZVOD* nadjen = (PROIZVOD*)bsearch(
            &id, p, (size_t)n, sizeof(PROIZVOD), usporediProizvodID);
        if (nadjen) {
            printf("%-5d %-30s %-20s %10.2f EUR\n",
                nadjen->id, nadjen->ime,
                nadjen->kategorija, nadjen->cijena);
            pron = 1;
        }
        break;
    }
    case PROIZVOD_PRETRAGA_IME: {
        char upit[50];
        printf("Dio imena: "); citajLiniju(upit, sizeof(upit));
        for (int i = 0; i < n; i++)
            if (strstr(p[i].ime, upit)) {
                printf("%-5d %-30s %-20s %10.2f EUR\n",
                    p[i].id, p[i].ime,
                    p[i].kategorija, p[i].cijena);
                pron = 1;
            }
        break;
    }
    case PROIZVOD_PRETRAGA_KATEGORIJA: {
        char upit[30];
        printf("Kategorija: "); citajLiniju(upit, sizeof(upit));
        for (int i = 0; i < n; i++)
            if (strstr(p[i].kategorija, upit)) {
                printf("%-5d %-30s %-20s %10.2f EUR\n",
                    p[i].id, p[i].ime,
                    p[i].kategorija, p[i].cijena);
                pron = 1;
            }
        break;
    }
    case PROIZVOD_PRETRAGA_CIJENA: {
        float mn, mx;
        printf("Min cijena: "); scanf("%f", &mn); ocistiBuffer();
        printf("Max cijena: "); scanf("%f", &mx); ocistiBuffer();
        for (int i = 0; i < n; i++)
            if (p[i].cijena >= mn && p[i].cijena <= mx) {
                printf("%-5d %-30s %-20s %10.2f EUR\n",
                    p[i].id, p[i].ime,
                    p[i].kategorija, p[i].cijena);
                pron = 1;
            }
        break;
    }
    default:
        printf("Nevazeci izbor.\n"); return;
    }

    if (!pron) printf("Nista pronadeno.\n");
}

/* ------------------------------------------------------------------ */
/*  Izbornik (tocke 10, 11)                                            */
/* ------------------------------------------------------------------ */

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
        printf("  8. Sortiraj proizvode po cijeni\n");
        printf("Izbor: ");
        scanf("%d", &izbor); ocistiBuffer();

        switch ((OpcijaProizvodi)izbor) {
        case PROIZVOD_DODAJ:
            dodajProizvod(datoteka);
            break;
        case PROIZVOD_UCITAJ:
            if (p) { free(p); p = NULL; }
            p = ucitajProizvode(datoteka, &n);
            printf(p ? "Ucitano %d proizvoda.\n"
                : "Datoteka prazna ili ne postoji.\n", n);
            break;
        case PROIZVOD_ISPISI:
            ispisiProizvode(p, n);
            break;
        case PROIZVOD_UREDI:
            urediProizvod(datoteka);
            if (p) { free(p); p = NULL; }
            p = ucitajProizvode(datoteka, &n);
            break;
        case PROIZVOD_OBRISI:
            obrisiProizvod(datoteka);
            if (p) { free(p); p = NULL; }
            p = ucitajProizvode(datoteka, &n);
            break;
        case PROIZVOD_PRETRAZI:
            pretraziProizvode(p, n);
            break;
        case PROIZVOD_SORTIRAJ:
            sortirajProizvode(p, n);
            printf("Proizvodi sortirani po cijeni.\n");
            ispisiProizvode(p, n);
            break;
        case PROIZVOD_POVRATAK:
            if (p) { free(p); p = NULL; n = 0; }
            return;
        default:
            printf("Nevazeci izbor.\n");
        }
    }
}