#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "zalihe.h"
#include "pomocnici.h"

/*  Static pomocne funkcije (tocka 6)*/

static int postojiProizvodID(ZALIHA* z, int n, int id) {
    if (!z || n <= 0) return 0;
    for (int i = 0; i < n; i++)
        if (z[i].proizvod_id == id) return 1;
    return 0;
}

/*  Komparatori za qsort i bsearch (tocke 23, 24, 26)*/

/* Pokazivac na funkciju – sortiranje po kolicini (rastuce) */
static int usporediKolicinu(const void* a, const void* b) {
    return ((const ZALIHA*)a)->kolicina -
        ((const ZALIHA*)b)->kolicina;
}

/* Komparator za bsearch po proizvod_id */
static int usporediZalihaID(const void* kljuc, const void* elem) {
    int id = *(const int*)kljuc;
    return id - ((const ZALIHA*)elem)->proizvod_id;
}


/*  Rekurzivni binarni search po proizvod_id (tocka 25)*/

static int binSearchZalihaRek(ZALIHA* z, int lijevo, int desno, int id) {
    if (lijevo > desno) return -1;
    int sredina = lijevo + (desno - lijevo) / 2;
    if (z[sredina].proizvod_id == id)   return sredina;
    if (z[sredina].proizvod_id < id)
        return binSearchZalihaRek(z, sredina + 1, desno, id);
    return binSearchZalihaRek(z, lijevo, sredina - 1, id);
}

/*CRUD funkcije*/

ZALIHA* ucitajZalihe(const char* datoteka, int* n) {
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

    ZALIHA* z = malloc((size_t)(*n) * sizeof(ZALIHA));  /* tocka 17 */
    if (!z) { ispisiGresku("malloc"); fclose(fp); *n = 0; return NULL; }

    if ((int)fread(z, sizeof(ZALIHA), (size_t)(*n), fp) != *n) {
        if (ferror(fp)) ispisiGresku("fread (data)");
        free(z); z = NULL; *n = 0; fclose(fp); return NULL;
    }

    fclose(fp);
    return z;
}

int spremiZalihe(const char* datoteka, ZALIHA* z, int n) {
    if (!datoteka) return 0;
    return atomicnoSpremi(datoteka, z, n, sizeof(ZALIHA));  /* tocka 21 */
}

void ispisiZalihe(ZALIHA* z, int n) {
    if (!z || n <= 0) {   /* tocka 14 */
        printf("Nema ucitanih zaliha. Koristite opciju 'Ucitaj zalihe'.\n");
        return;
    }
    printf("\n%-10s %-25s %-10s %-12s %-25s\n",
        "ProizvodID", "Naziv", "Kolicina", "Min.Kolicina", "Lokacija");
    printf("%-10s %-25s %-10s %-12s %-25s\n",
        "----------", "-------------------------",
        "----------", "------------", "-------------------------");
    for (int i = 0; i < n; i++) {
        const char* upoz = (z[i].kolicina <= z[i].min_kolicina)
            ? " [!NISKA ZALIHA]" : "";
        printf("%-10d %-25s %-10d %-12d %-25s%s\n",
            z[i].proizvod_id, z[i].naziv,
            z[i].kolicina, z[i].min_kolicina,
            z[i].lokacija, upoz);
    }
    printf("Ukupno: %d stavk(e) u zalihama\n", n);
}

void prikaziNiskuZalihu(ZALIHA* z, int n) {
    if (!z || n <= 0) { printf("Nema ucitanih zaliha.\n"); return; }

    int pronadeno = 0;
    printf("\n=== UPOZORENJA – NISKA ZALIHA ===\n");
    for (int i = 0; i < n; i++) {
        if (z[i].kolicina <= z[i].min_kolicina) {
            printf("  [!] ProizvodID %-4d | %-25s | Stanje: %d | Min: %d | %s\n",
                z[i].proizvod_id, z[i].naziv,
                z[i].kolicina, z[i].min_kolicina, z[i].lokacija);
            pronadeno++;
        }
    }
    if (!pronadeno) printf("  Sve zalihe su iznad minimalne razine.\n");
    else            printf("\nUkupno upozorenja: %d\n", pronadeno);
}

void dodajZalihu(const char* datoteka) {
    if (!datoteka) return;

    int     n = 0;
    ZALIHA* z = ucitajZalihe(datoteka, &n);

    ZALIHA novi;
    memset(&novi, 0, sizeof(ZALIHA));

    printf("ID proizvoda: ");
    scanf("%d", &novi.proizvod_id); ocistiBuffer();

    if (postojiProizvodID(z, n, novi.proizvod_id)) {
        printf("Zaliha za proizvod ID %d vec postoji. "
            "Koristite opciju 'Uredi'.\n", novi.proizvod_id);
        if (z) { free(z); z = NULL; }
        return;
    }

    printf("Naziv proizvoda       : "); citajLiniju(novi.naziv, sizeof(novi.naziv));

    printf("Kolicina na stanju    : ");
    while (scanf("%d", &novi.kolicina) != 1 || novi.kolicina < 0) {
        printf("Unesite pozitivan broj: "); ocistiBuffer();
    }
    ocistiBuffer();

    printf("Minimalna kolicina    : ");
    while (scanf("%d", &novi.min_kolicina) != 1 || novi.min_kolicina < 0) {
        printf("Unesite pozitivan broj: "); ocistiBuffer();
    }
    ocistiBuffer();

    printf("Lokacija u skladistu  : "); citajLiniju(novi.lokacija, sizeof(novi.lokacija));

    ZALIHA* tmp = realloc(z, (size_t)(n + 1) * sizeof(ZALIHA));  /* tocka 17 */
    if (!tmp) {
        ispisiGresku("realloc");
        free(z); z = NULL; return;
    }
    z = tmp;
    z[n++] = novi;

    if (spremiZalihe(datoteka, z, n))
        printf("Zaliha uspjesno dodana za proizvod ID %d.\n", novi.proizvod_id);
    else
        printf("Greska pri spremanju.\n");

    free(z); z = NULL;
}

void urediZalihu(const char* datoteka) {
    if (!datoteka) return;

    int     n = 0;
    ZALIHA* z = ucitajZalihe(datoteka, &n);
    if (!z) { printf("Nema podataka za uredivanje.\n"); return; }

    ispisiZalihe(z, n);

    int id;
    printf("\nID proizvoda za uredivanje zalihe: ");
    scanf("%d", &id); ocistiBuffer();

    /* tocka 25 – rekurzivni binary search */
    int idx = binSearchZalihaRek(z, 0, n - 1, id);
    if (idx == -1) {
        printf("Zaliha za proizvod ID %d nije pronadena.\n", id);
        free(z); z = NULL; return;
    }

    printf("\n--- Uredivanje zalihe: %s ---\n", z[idx].naziv);
    char buf[100];

    printf("Naziv [%s]: ", z[idx].naziv);
    citajLiniju(buf, sizeof(buf));
    if (buf[0] != '\0') sigurnoKopiraj(z[idx].naziv, buf, sizeof(z[idx].naziv));

    printf("Kolicina [%d] (-1 za zadrzati): ", z[idx].kolicina);
    int v; scanf("%d", &v); ocistiBuffer();
    if (v >= 0) z[idx].kolicina = v;

    printf("Minimalna kolicina [%d] (-1 za zadrzati): ", z[idx].min_kolicina);
    scanf("%d", &v); ocistiBuffer();
    if (v >= 0) z[idx].min_kolicina = v;

    printf("Lokacija [%s]: ", z[idx].lokacija);
    citajLiniju(buf, sizeof(buf));
    if (buf[0] != '\0') sigurnoKopiraj(z[idx].lokacija, buf, sizeof(z[idx].lokacija));

    if (spremiZalihe(datoteka, z, n))
        printf("Zaliha uspjesno uredena.\n");
    else
        printf("Greska pri spremanju.\n");

    free(z); z = NULL;
}

void obrisiZalihu(const char* datoteka) {
    if (!datoteka) return;

    int     n = 0;
    ZALIHA* z = ucitajZalihe(datoteka, &n);
    if (!z) { printf("Nema podataka za brisanje.\n"); return; }

    ispisiZalihe(z, n);

    int id;
    printf("\nID proizvoda ciju zalihu zelite obrisati: ");
    scanf("%d", &id); ocistiBuffer();

    int idx = -1;
    for (int i = 0; i < n; i++)
        if (z[i].proizvod_id == id) { idx = i; break; }

    if (idx == -1) {
        printf("Zaliha za proizvod ID %d nije pronadena.\n", id);
        free(z); z = NULL; return;
    }

    char potvrda[10];
    printf("Brisanje zalihe '%s'. Potvrdite (da/ne): ", z[idx].naziv);
    citajLiniju(potvrda, sizeof(potvrda));

    if (strcmp(potvrda, "da") != 0) {
        printf("Brisanje odustano.\n");
        free(z); z = NULL; return;
    }

    for (int i = idx; i < n - 1; i++)
        z[i] = z[i + 1];
    n--;
    memset(&z[n], 0, sizeof(ZALIHA));   /* tocka 18 */

    if (spremiZalihe(datoteka, z, n))
        printf("Zaliha uspjesno obrisana.\n");
    else
        printf("Greska pri spremanju.\n");

    free(z); z = NULL;
}

/*  Sortiranje (tocke 23, 26)*/

void sortirajZalihe(ZALIHA* z, int n) {
    if (!z || n <= 1) return;
    /* tocka 23 – qsort, tocka 26 – pokazivac na funkciju usporediKolicinu */
    qsort(z, (size_t)n, sizeof(ZALIHA), usporediKolicinu);
}

/*  Izbornik (tocke 10, 11)*/

void izbornikZalihe(const char* datoteka) {
    int     izbor;
    ZALIHA* z = NULL;
    int     n = 0;

    while (1) {
        printf("\n--- UPRAVLJANJE ZALIHAMA ---\n");
        printf("  1. Dodaj zalihu\n");
        printf("  2. Ucitaj zalihe iz datoteke\n");
        printf("  3. Ispisi sve zalihe\n");
        printf("  4. Uredi zalihu\n");
        printf("  5. Obrisi zalihu\n");
        printf("  6. Prikazi niske zalihe (upozorenja)\n");
        printf("  7. Povratak\n");
        printf("  8. Sortiraj zalihe po kolicini\n");
        printf("Izbor: ");
        scanf("%d", &izbor); ocistiBuffer();

        switch ((OpcijaZalihe)izbor) {
        case ZALIHA_DODAJ:
            dodajZalihu(datoteka);
            break;
        case ZALIHA_UCITAJ:
            if (z) { free(z); z = NULL; }
            z = ucitajZalihe(datoteka, &n);
            printf(z ? "Ucitano %d stavki zaliha.\n"
                : "Datoteka prazna ili ne postoji.\n", n);
            break;
        case ZALIHA_ISPISI:
            ispisiZalihe(z, n);
            break;
        case ZALIHA_UREDI:
            urediZalihu(datoteka);
            if (z) { free(z); z = NULL; }
            z = ucitajZalihe(datoteka, &n);
            break;
        case ZALIHA_OBRISI:
            obrisiZalihu(datoteka);
            if (z) { free(z); z = NULL; }
            z = ucitajZalihe(datoteka, &n);
            break;
        case ZALIHA_NISKA:
            prikaziNiskuZalihu(z, n);
            break;
        case ZALIHA_SORTIRAJ:
            sortirajZalihe(z, n);
            printf("Zalihe sortirane po kolicini.\n");
            ispisiZalihe(z, n);
            break;
        case ZALIHA_POVRATAK:
            if (z) { free(z); z = NULL; n = 0; }
            return;
        default:
            printf("Nevazeci izbor.\n");
        }
    }
}