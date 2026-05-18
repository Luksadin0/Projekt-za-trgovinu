#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zalihe.h"
#include "pomocnici.h"

typedef enum {
    ZALIHA_DODAJ = 1,
    ZALIHA_UCITAJ = 2,
    ZALIHA_ISPISI = 3,
    ZALIHA_UREDI = 4,
    ZALIHA_OBRISI = 5,
    ZALIHA_NISKA = 6,
    ZALIHA_POVRATAK = 7
} OpcijaZalihe;

static int postojiProizvodID(ZALIHA* z, int n, int id) {
    for (int i = 0; i < n; i++)
        if (z[i].proizvod_id == id) return 1;
    return 0;
}


ZALIHA* ucitajZalihe(const char* datoteka, int* n) {
    FILE* fp = fopen(datoteka, "rb");
    if (!fp) { *n = 0; return NULL; }

    if (fread(n, sizeof(int), 1, fp) != 1 || *n <= 0) {
        fclose(fp); *n = 0; return NULL;
    }

    ZALIHA* z = malloc(*n * sizeof(ZALIHA));
    if (!z) { fclose(fp); *n = 0; return NULL; }

    fread(z, sizeof(ZALIHA), *n, fp);
    fclose(fp);
    return z;
}

int spremiZalihe(const char* datoteka, ZALIHA* z, int n) {
    FILE* fp = fopen(datoteka, "wb");
    if (!fp) return 0;
    fwrite(&n, sizeof(int), 1, fp);
    if (n > 0) fwrite(z, sizeof(ZALIHA), n, fp);
    fclose(fp);
    return 1;
}

void ispisiZalihe(ZALIHA* z, int n) {
    if (!z || n == 0) {
        printf("Nema ucitanih zaliha. Koristite opciju 'Ucitaj zalihe'.\n");
        return;
    }
    printf("\n%-10s %-25s %-10s %-12s %-25s %s\n",
        "ProzvodID", "Naziv", "Kolicina", "Min.Kolicina", "Lokacija", "");
    printf("%-10s %-25s %-10s %-12s %-25s\n",
        "----------", "-------------------------", "----------",
        "------------", "-------------------------");
    for (int i = 0; i < n; i++) {
        const char* upoz = (z[i].kolicina <= z[i].min_kolicina) ? " [!NISKA ZALIHA]" : "";
        printf("%-10d %-25s %-10d %-12d %-25s%s\n",
            z[i].proizvod_id, z[i].naziv,
            z[i].kolicina, z[i].min_kolicina,
            z[i].lokacija, upoz);
    }
    printf("Ukupno: %d stavk(e) u zalihama\n", n);
}

void prikaziNiskuZalihu(ZALIHA* z, int n) {
    if (!z || n == 0) {
        printf("Nema ucitanih zaliha.\n");
        return;
    }
    int pronadeno = 0;
    printf("\n=== UPOZORENJA - NISKA ZALIHA ===\n");
    for (int i = 0; i < n; i++) {
        if (z[i].kolicina <= z[i].min_kolicina) {
            printf("  [!] Proizvod ID %-4d | %-25s | Na stanju: %d | Minimum: %d | Lokacija: %s\n",
                z[i].proizvod_id, z[i].naziv,
                z[i].kolicina, z[i].min_kolicina, z[i].lokacija);
            pronadeno++;
        }
    }
    if (!pronadeno) printf("  Sve zalihe su iznad minimalne razine.\n");
    else printf("\nUkupno upozorenja: %d\n", pronadeno);
}

void dodajZalihu(const char* datoteka) {
    int     n = 0;
    ZALIHA* z = ucitajZalihe(datoteka, &n);

    ZALIHA novi;

    printf("ID proizvoda (mora odgovarati postojecem proizvodu): ");
    scanf("%d", &novi.proizvod_id); ocistiBuffer();

    if (postojiProizvodID(z, n, novi.proizvod_id)) {
        printf("Zaliha za proizvod ID %d vec postoji. Koristite opciju 'Uredi'.\n",
            novi.proizvod_id);
        if (z) free(z);
        return;
    }

    printf("Naziv proizvoda : "); citajLiniju(novi.naziv, sizeof(novi.naziv));
    printf("Kolicina na stanju: ");
    while (scanf("%d", &novi.kolicina) != 1 || novi.kolicina < 0) {
        printf("Unesite pozitivan broj: "); ocistiBuffer();
    }
    ocistiBuffer();
    printf("Minimalna kolicina (upozorenje): ");
    while (scanf("%d", &novi.min_kolicina) != 1 || novi.min_kolicina < 0) {
        printf("Unesite pozitivan broj: "); ocistiBuffer();
    }
    ocistiBuffer();
    printf("Lokacija u skladistu: "); citajLiniju(novi.lokacija, sizeof(novi.lokacija));

    ZALIHA* tmp = realloc(z, (n + 1) * sizeof(ZALIHA));
    if (!tmp) { free(z); printf("Greska pri alokaciji.\n"); return; }
    z = tmp;
    z[n++] = novi;

    if (spremiZalihe(datoteka, z, n))
        printf("Zaliha uspjesno dodana za proizvod ID %d.\n", novi.proizvod_id);
    else
        printf("Greska pri spremanju.\n");
    free(z);
}

void urediZalihu(const char* datoteka) {
    int     n = 0;
    ZALIHA* z = ucitajZalihe(datoteka, &n);
    if (!z) { printf("Nema podataka za uredivanje.\n"); return; }

    ispisiZalihe(z, n);

    int id;
    printf("\nID proizvoda za uredivanje zalihe: ");
    scanf("%d", &id); ocistiBuffer();

    int idx = -1;
    for (int i = 0; i < n; i++)
        if (z[i].proizvod_id == id) { idx = i; break; }

    if (idx == -1) {
        printf("Zaliha za proizvod ID %d nije pronadena.\n", id);
        free(z); return;
    }

    printf("\n--- Uredivanje zalihe: %s ---\n", z[idx].naziv);

    char buf[100];

    printf("Naziv [%s]: ", z[idx].naziv);
    citajLiniju(buf, sizeof(buf));
    if (buf[0] != '\0') strncpy(z[idx].naziv, buf, sizeof(z[idx].naziv) - 1);

    printf("Kolicina [%d] (-1 za zadrzati): ", z[idx].kolicina);
    int v; scanf("%d", &v); ocistiBuffer();
    if (v >= 0) z[idx].kolicina = v;

    printf("Minimalna kolicina [%d] (-1 za zadrzati): ", z[idx].min_kolicina);
    scanf("%d", &v); ocistiBuffer();
    if (v >= 0) z[idx].min_kolicina = v;

    printf("Lokacija [%s]: ", z[idx].lokacija);
    citajLiniju(buf, sizeof(buf));
    if (buf[0] != '\0') strncpy(z[idx].lokacija, buf, sizeof(z[idx].lokacija) - 1);

    if (spremiZalihe(datoteka, z, n))
        printf("Zaliha uspjesno uredena.\n");
    else
        printf("Greska pri spremanju.\n");
    free(z);
}

void obrisiZalihu(const char* datoteka) {
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
        free(z); return;
    }

    char potvrda[10];
    printf("Brisanje zalihe '%s'. Potvrdite (da/ne): ", z[idx].naziv);
    citajLiniju(potvrda, sizeof(potvrda));

    if (strcmp(potvrda, "da") != 0) {
        printf("Brisanje odustano.\n");
        free(z); return;
    }

    for (int i = idx; i < n - 1; i++)
        z[i] = z[i + 1];
    n--;

    if (spremiZalihe(datoteka, z, n))
        printf("Zaliha uspjesno obrisana.\n");
    else
        printf("Greska pri spremanju.\n");
    free(z);
}



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
        printf("Izbor: ");
        scanf("%d", &izbor); ocistiBuffer();

        switch ((OpcijaZalihe)izbor) {
        case ZALIHA_DODAJ: dodajZalihu(datoteka); break;
        case ZALIHA_UCITAJ:
            if (z) free(z);
            z = ucitajZalihe(datoteka, &n);
            printf(z ? "Ucitano %d stavki zaliha.\n"
                : "Datoteka prazna ili ne postoji.\n", n);
            break;
        case ZALIHA_ISPISI: ispisiZalihe(z, n); break;
        case ZALIHA_UREDI:
            urediZalihu(datoteka);
            if (z) { free(z); z = ucitajZalihe(datoteka, &n); }
            break;
        case ZALIHA_OBRISI:
            obrisiZalihu(datoteka);
            if (z) { free(z); z = ucitajZalihe(datoteka, &n); }
            break;
        case ZALIHA_NISKA: prikaziNiskuZalihu(z, n); break;
        case ZALIHA_POVRATAK:
            if (z) { free(z); z = NULL; n = 0; }
            return;
        default:
            printf("Nevazeci izbor.\n");
        }
    }
}