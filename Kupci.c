#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "kupci.h"
#include "pomocnici.h"

/* ------------------------------------------------------------------ */
/*  Static pomocne funkcije (tocka 6)                                  */
/* ------------------------------------------------------------------ */

static int sljedeciID(KUPAC* k, int n) {
    int maks = 0;
    for (int i = 0; i < n; i++)
        if (k[i].id > maks) maks = k[i].id;
    return maks + 1;
}

/* ------------------------------------------------------------------ */
/*  Komparatori za qsort i bsearch (tocke 23, 24, 26)                  */
/* ------------------------------------------------------------------ */

/* Pokazivac na funkciju – komparator po prezimenu */
static int usporediPrezime(const void* a, const void* b) {
    return strcmp(((const KUPAC*)a)->prezime,
        ((const KUPAC*)b)->prezime);
}

/* Komparator za bsearch po ID-u */
static int usporediID(const void* kljuc, const void* elem) {
    int id = *(const int*)kljuc;
    return id - ((const KUPAC*)elem)->id;
}

/* ------------------------------------------------------------------ */
/*  Rekurzivni binarni search po ID-u (tocka 25)                       */
/* ------------------------------------------------------------------ */
static int binSearchRekurzivno(KUPAC* k, int lijevo, int desno,
    int trazeniID) {
    if (lijevo > desno) return -1;
    int sredina = lijevo + (desno - lijevo) / 2;
    if (k[sredina].id == trazeniID) return sredina;
    if (k[sredina].id < trazeniID)
        return binSearchRekurzivno(k, sredina + 1, desno, trazeniID);
    return binSearchRekurzivno(k, lijevo, sredina - 1, trazeniID);
}

/* ------------------------------------------------------------------ */
/*  CRUD funkcije                                                       */
/* ------------------------------------------------------------------ */

KUPAC* ucitajKupce(const char* datoteka, int* n) {
    if (!datoteka || !n) return NULL;

    FILE* fp = fopen(datoteka, "rb");
    if (!fp) {
        /* Datoteka ne mora postojati pri prvom pokretanju */
        *n = 0;
        return NULL;
    }

    /* tocka 20 – rewind za osiguranje pocetka */
    rewind(fp);

    if (fread(n, sizeof(int), 1, fp) != 1 || *n <= 0) {
        /* tocka 22 – ferror / feof */
        if (ferror(fp))
            ispisiGresku("fread (count)");
        fclose(fp);
        *n = 0;
        return NULL;
    }

    /* tocka 20 – ftell za provjeru pozicije */
    long pos = ftell(fp);
    if (pos < 0) {
        ispisiGresku("ftell");
        fclose(fp);
        *n = 0;
        return NULL;
    }

    /* tocka 16 – malloc (tocka 17) */
    KUPAC* k = malloc((size_t)(*n) * sizeof(KUPAC));
    if (!k) {
        ispisiGresku("malloc");
        fclose(fp);
        *n = 0;
        return NULL;
    }

    size_t procitano = fread(k, sizeof(KUPAC), (size_t)(*n), fp);
    if ((int)procitano != *n) {
        /* tocka 22 – ferror */
        if (ferror(fp)) ispisiGresku("fread (data)");
        free(k);    /* tocka 18 – sigurno oslobadanje */
        k = NULL;
        *n = 0;
        fclose(fp);
        return NULL;
    }

    fclose(fp);
    return k;
}

int spremiKupce(const char* datoteka, KUPAC* k, int n) {
    if (!datoteka) return 0;
    /* tocka 21 – atomicno spremanje kroz pomocnici.h */
    return atomicnoSpremi(datoteka, k, n, sizeof(KUPAC));
}

void ispisiKupce(KUPAC* k, int n) {
    /* tocka 14 – zastita parametara */
    if (!k || n <= 0) {
        printf("Nema ucitanih kupaca. Koristite opciju 'Ucitaj kupce'.\n");
        return;
    }
    printf("\n%-5s %-20s %-20s %-30s %-15s\n",
        "ID", "Ime", "Prezime", "Email", "Telefon");
    printf("%-5s %-20s %-20s %-30s %-15s\n",
        "-----", "--------------------", "--------------------",
        "------------------------------", "---------------");
    for (int i = 0; i < n; i++)
        printf("%-5d %-20s %-20s %-30s %-15s\n",
            k[i].id, k[i].ime, k[i].prezime,
            k[i].email, k[i].telefon);
    printf("Ukupno: %d kupac(a)\n", n);
}

void dodajKupca(const char* datoteka) {
    if (!datoteka) return;

    int    n = 0;
    KUPAC* k = ucitajKupce(datoteka, &n);

    KUPAC novi;
    memset(&novi, 0, sizeof(KUPAC));   /* tocka 17 – calloc alternativa */
    novi.id = sljedeciID(k, n);

    printf("Ime         : "); citajLiniju(novi.ime, sizeof(novi.ime));
    printf("Prezime     : "); citajLiniju(novi.prezime, sizeof(novi.prezime));
    printf("Email       : "); citajLiniju(novi.email, sizeof(novi.email));
    printf("Telefon     : "); citajLiniju(novi.telefon, sizeof(novi.telefon));
    printf("Adresa      : "); citajLiniju(novi.adresa, sizeof(novi.adresa));

    /* tocka 17 – realloc */
    KUPAC* tmp = realloc(k, (size_t)(n + 1) * sizeof(KUPAC));
    if (!tmp) {
        ispisiGresku("realloc");
        free(k); k = NULL;   /* tocka 18 */
        return;
    }
    k = tmp;
    k[n++] = novi;

    if (spremiKupce(datoteka, k, n))
        printf("Kupac uspjesno dodan! (ID: %d)\n", novi.id);
    else
        printf("Greska pri spremanju datoteke.\n");

    free(k); k = NULL;   /* tocka 18 */
}

void urediKupca(const char* datoteka) {
    if (!datoteka) return;

    int    n = 0;
    KUPAC* k = ucitajKupce(datoteka, &n);
    if (!k) { printf("Nema podataka za uredivanje.\n"); return; }

    ispisiKupce(k, n);

    int id;
    printf("\nID kupca za uredivanje: ");
    scanf("%d", &id); ocistiBuffer();

    /* tocka 25 – rekurzivni binarni search (zahtijeva sortirani niz) */
    sortirajKupce(k, n);
    int idx = binSearchRekurzivno(k, 0, n - 1, id);

    if (idx == -1) {
        printf("Kupac s ID-om %d nije pronaden.\n", id);
        free(k); k = NULL; return;
    }

    char buf[100];

    printf("Ime [%s]: ", k[idx].ime);     citajLiniju(buf, sizeof(buf));
    if (buf[0] != '\0') sigurnoKopiraj(k[idx].ime, buf, sizeof(k[idx].ime));

    printf("Prezime [%s]: ", k[idx].prezime); citajLiniju(buf, sizeof(buf));
    if (buf[0] != '\0') sigurnoKopiraj(k[idx].prezime, buf, sizeof(k[idx].prezime));

    printf("Email [%s]: ", k[idx].email);   citajLiniju(buf, sizeof(buf));
    if (buf[0] != '\0') sigurnoKopiraj(k[idx].email, buf, sizeof(k[idx].email));

    printf("Telefon [%s]: ", k[idx].telefon); citajLiniju(buf, sizeof(buf));
    if (buf[0] != '\0') sigurnoKopiraj(k[idx].telefon, buf, sizeof(k[idx].telefon));

    printf("Adresa [%s]: ", k[idx].adresa);  citajLiniju(buf, sizeof(buf));
    if (buf[0] != '\0') sigurnoKopiraj(k[idx].adresa, buf, sizeof(k[idx].adresa));

    if (spremiKupce(datoteka, k, n))
        printf("Kupac ID %d uspjesno ureden.\n", id);
    else
        printf("Greska pri spremanju.\n");

    free(k); k = NULL;
}

void obrisiKupca(const char* datoteka) {
    if (!datoteka) return;

    int    n = 0;
    KUPAC* k = ucitajKupce(datoteka, &n);
    if (!k) { printf("Nema podataka za brisanje.\n"); return; }

    ispisiKupce(k, n);

    int id;
    printf("\nID kupca za brisanje: ");
    scanf("%d", &id); ocistiBuffer();

    int idx = -1;
    for (int i = 0; i < n; i++)
        if (k[i].id == id) { idx = i; break; }

    if (idx == -1) {
        printf("Kupac s ID-om %d nije pronaden.\n", id);
        free(k); k = NULL; return;
    }

    char potvrda[10];
    printf("Brisanje kupca '%s %s'. Potvrdite (da/ne): ",
        k[idx].ime, k[idx].prezime);
    citajLiniju(potvrda, sizeof(potvrda));

    if (strcmp(potvrda, "da") != 0) {
        printf("Brisanje odustano.\n");
        free(k); k = NULL; return;
    }

    /* Pomicanje elemenata (brisanje bez realloc-a) */
    for (int i = idx; i < n - 1; i++)
        k[i] = k[i + 1];
    n--;

    /* Anuliranje zadnjeg (viseceg) elementa (tocka 18) */
    memset(&k[n], 0, sizeof(KUPAC));

    if (spremiKupce(datoteka, k, n))
        printf("Kupac uspjesno obrisan.\n");
    else
        printf("Greska pri spremanju.\n");

    free(k); k = NULL;
}

/* ------------------------------------------------------------------ */
/*  Sortiranje (tocka 23) i pretraga (tocka 24)                        */
/* ------------------------------------------------------------------ */

void sortirajKupce(KUPAC* k, int n) {
    if (!k || n <= 1) return;
    /* tocka 23 – qsort, tocka 26 – pokazivac na funkciju usporediPrezime */
    qsort(k, (size_t)n, sizeof(KUPAC), usporediPrezime);
}

/* tocka 24 – bsearch po ID-u (niz mora biti sortiran po ID-u) */
KUPAC* nadjiKupcaID(KUPAC* k, int n, int trazeniID) {
    if (!k || n <= 0) return NULL;
    return (KUPAC*)bsearch(&trazeniID, k, (size_t)n,
        sizeof(KUPAC), usporediID);
}

void pretraziKupce(KUPAC* k, int n) {
    if (!k || n <= 0) {
        printf("Nema ucitanih podataka. Koristite opciju 'Ucitaj kupce'.\n");
        return;
    }

    int opcija, pron = 0;
    printf("\nPretraga po:\n");
    printf("  1. ID-u\n  2. Prezimenu\n  3. Emailu\n  4. Telefonskom broju\n");
    printf("Izbor: ");
    scanf("%d", &opcija); ocistiBuffer();

    printf("\n%-5s %-20s %-20s %-30s %-15s\n",
        "ID", "Ime", "Prezime", "Email", "Telefon");
    printf("%-5s %-20s %-20s %-30s %-15s\n",
        "-----", "--------------------", "--------------------",
        "------------------------------", "---------------");

    switch ((OpcijaKupacPretraga)opcija) {
    case KUPAC_PRETRAGA_ID: {
        int id;
        printf("ID: "); scanf("%d", &id); ocistiBuffer();
        /* tocka 24 – bsearch za brzu pretragu po ID-u */
        KUPAC* nadjen = nadjiKupcaID(k, n, id);
        if (nadjen) {
            printf("%-5d %-20s %-20s %-30s %-15s\n",
                nadjen->id, nadjen->ime, nadjen->prezime,
                nadjen->email, nadjen->telefon);
            printf("Adresa: %s\n", nadjen->adresa);
            pron = 1;
        }
        break;
    }
    case KUPAC_PRETRAGA_PREZIME: {
        char upit[50];
        printf("Dio prezimena: "); citajLiniju(upit, sizeof(upit));
        for (int i = 0; i < n; i++)
            if (strstr(k[i].prezime, upit)) {
                printf("%-5d %-20s %-20s %-30s %-15s\n",
                    k[i].id, k[i].ime, k[i].prezime,
                    k[i].email, k[i].telefon);
                pron = 1;
            }
        break;
    }
    case KUPAC_PRETRAGA_EMAIL: {
        char upit[80];
        printf("Email: "); citajLiniju(upit, sizeof(upit));
        for (int i = 0; i < n; i++)
            if (strstr(k[i].email, upit)) {
                printf("%-5d %-20s %-20s %-30s %-15s\n",
                    k[i].id, k[i].ime, k[i].prezime,
                    k[i].email, k[i].telefon);
                pron = 1;
            }
        break;
    }
    case KUPAC_PRETRAGA_TELEFON: {
        char upit[20];
        printf("Telefon: "); citajLiniju(upit, sizeof(upit));
        for (int i = 0; i < n; i++)
            if (strstr(k[i].telefon, upit)) {
                printf("%-5d %-20s %-20s %-30s %-15s\n",
                    k[i].id, k[i].ime, k[i].prezime,
                    k[i].email, k[i].telefon);
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

void izbornikKupci(const char* datoteka) {
    int    izbor;
    KUPAC* k = NULL;
    int    n = 0;

    while (1) {
        printf("\n--- UPRAVLJANJE KUPCIMA ---\n");
        printf("  1. Dodaj novog kupca\n");
        printf("  2. Ucitaj kupce iz datoteke\n");
        printf("  3. Ispisi sve kupce\n");
        printf("  4. Uredi kupca\n");
        printf("  5. Obrisi kupca\n");
        printf("  6. Pretrazi kupce\n");
        printf("  7. Povratak\n");
        printf("  8. Sortiraj kupce po prezimenu\n");
        printf("Izbor: ");
        scanf("%d", &izbor); ocistiBuffer();

        switch ((OpcijaKupci)izbor) {
        case KUPAC_DODAJ:
            dodajKupca(datoteka);
            break;
        case KUPAC_UCITAJ:
            if (k) { free(k); k = NULL; }   /* tocka 18 */
            k = ucitajKupce(datoteka, &n);
            printf(k ? "Ucitano %d kupaca.\n"
                : "Datoteka prazna ili ne postoji.\n", n);
            break;
        case KUPAC_ISPISI:
            ispisiKupce(k, n);
            break;
        case KUPAC_UREDI:
            urediKupca(datoteka);
            if (k) { free(k); k = NULL; }
            k = ucitajKupce(datoteka, &n);
            break;
        case KUPAC_OBRISI:
            obrisiKupca(datoteka);
            if (k) { free(k); k = NULL; }
            k = ucitajKupce(datoteka, &n);
            break;
        case KUPAC_PRETRAZI:
            pretraziKupce(k, n);
            break;
        case KUPAC_SORTIRAJ:
            sortirajKupce(k, n);
            printf("Kupci sortirani po prezimenu.\n");
            ispisiKupce(k, n);
            break;
        case KUPAC_POVRATAK:
            if (k) { free(k); k = NULL; n = 0; }  /* tocka 18 */
            return;
        default:
            printf("Nevazeci izbor.\n");
        }
    }
}