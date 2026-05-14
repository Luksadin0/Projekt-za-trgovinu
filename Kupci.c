#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kupci.h"
#include "pomocnici.h"


static int sljedeciID(KUPAC* k, int n) {
    int maks = 0;
    for (int i = 0; i < n; i++)
        if (k[i].id > maks) maks = k[i].id;
    return maks + 1;
}


KUPAC* ucitajKupce(const char* datoteka, int* n) {
    FILE* fp = fopen(datoteka, "rb");
    if (!fp) { *n = 0; return NULL; }

    if (fread(n, sizeof(int), 1, fp) != 1 || *n <= 0) {
        fclose(fp); *n = 0; return NULL;
    }

    KUPAC* k = malloc(*n * sizeof(KUPAC));
    if (!k) { fclose(fp); *n = 0; return NULL; }

    fread(k, sizeof(KUPAC), *n, fp);
    fclose(fp);
    return k;
}

int spremiKupce(const char* datoteka, KUPAC* k, int n) {
    FILE* fp = fopen(datoteka, "wb");
    if (!fp) return 0;
    fwrite(&n, sizeof(int), 1, fp);
    if (n > 0) fwrite(k, sizeof(KUPAC), n, fp);
    fclose(fp);
    return 1;
}

void ispisiKupce(KUPAC* k, int n) {
    if (!k || n == 0) {
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
            k[i].id, k[i].ime, k[i].prezime, k[i].email, k[i].telefon);
    printf("Ukupno: %d kupac(a)\n", n);
}

void dodajKupca(const char* datoteka) {
    int    n = 0;
    KUPAC* k = ucitajKupce(datoteka, &n);

    KUPAC novi;
    novi.id = sljedeciID(k, n);

    printf("Ime         : "); citajLiniju(novi.ime, sizeof(novi.ime));
    printf("Prezime     : "); citajLiniju(novi.prezime, sizeof(novi.prezime));
    printf("Email       : "); citajLiniju(novi.email, sizeof(novi.email));
    printf("Telefon     : "); citajLiniju(novi.telefon, sizeof(novi.telefon));
    printf("Adresa      : "); citajLiniju(novi.adresa, sizeof(novi.adresa));

    KUPAC* tmp = realloc(k, (n + 1) * sizeof(KUPAC));
    if (!tmp) { free(k); printf("Greska pri alokaciji memorije.\n"); return; }
    k = tmp;
    k[n++] = novi;

    if (spremiKupce(datoteka, k, n))
        printf("Kupac uspjesno dodan! (ID: %d)\n", novi.id);
    else
        printf("Greska pri spremanju datoteke.\n");
    free(k);
}

void urediKupca(const char* datoteka) {
    int    n = 0;
    KUPAC* k = ucitajKupce(datoteka, &n);
    if (!k) { printf("Nema podataka za uredivanje.\n"); return; }

    ispisiKupce(k, n);

    int id;
    printf("\nID kupca za uredivanje: ");
    scanf("%d", &id); ocistiBuffer();

    int idx = -1;
    for (int i = 0; i < n; i++)
        if (k[i].id == id) { idx = i; break; }

    if (idx == -1) {
        printf("Kupac s ID-om %d nije pronaden.\n", id);
        free(k); return;
    }

    char buf[100];

    printf("Ime [%s]: ", k[idx].ime);     citajLiniju(buf, sizeof(buf));
    if (buf[0] != '\0') strncpy(k[idx].ime, buf, sizeof(k[idx].ime) - 1);

    printf("Prezime [%s]: ", k[idx].prezime);  citajLiniju(buf, sizeof(buf));
    if (buf[0] != '\0') strncpy(k[idx].prezime, buf, sizeof(k[idx].prezime) - 1);

    printf("Email [%s]: ", k[idx].email);    citajLiniju(buf, sizeof(buf));
    if (buf[0] != '\0') strncpy(k[idx].email, buf, sizeof(k[idx].email) - 1);

    printf("Telefon [%s]: ", k[idx].telefon);  citajLiniju(buf, sizeof(buf));
    if (buf[0] != '\0') strncpy(k[idx].telefon, buf, sizeof(k[idx].telefon) - 1);

    printf("Adresa [%s]: ", k[idx].adresa);   citajLiniju(buf, sizeof(buf));
    if (buf[0] != '\0') strncpy(k[idx].adresa, buf, sizeof(k[idx].adresa) - 1);

    if (spremiKupce(datoteka, k, n))
        printf("Kupac ID %d uspjesno ureden.\n", id);
    else
        printf("Greska pri spremanju.\n");
    free(k);
}

void obrisiKupca(const char* datoteka) {
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
        free(k); return;
    }

    char potvrda[10];
    printf("Brisanje kupca '%s %s'. Potvrdite (da/ne): ",
        k[idx].ime, k[idx].prezime);
    citajLiniju(potvrda, sizeof(potvrda));

    if (strcmp(potvrda, "da") != 0) {
        printf("Brisanje odustano.\n");
        free(k); return;
    }

    for (int i = idx; i < n - 1; i++)
        k[i] = k[i + 1];
    n--;

    if (spremiKupce(datoteka, k, n))
        printf("Kupac uspjesno obrisan.\n");
    else
        printf("Greska pri spremanju.\n");
    free(k);
}

void pretraziKupce(KUPAC* k, int n) {
    if (!k || n == 0) {
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

    switch (opcija) {
    case 1: {
        int id; printf("ID: "); scanf("%d", &id); ocistiBuffer();
        for (int i = 0; i < n; i++)
            if (k[i].id == id) {
                printf("%-5d %-20s %-20s %-30s %-15s\n",
                    k[i].id, k[i].ime, k[i].prezime, k[i].email, k[i].telefon);
                printf("Adresa: %s\n", k[i].adresa);
                pron = 1; break;
            }
        break;
    }
    case 2: {
        char upit[50]; printf("Dio prezimena: "); citajLiniju(upit, sizeof(upit));
        for (int i = 0; i < n; i++)
            if (strstr(k[i].prezime, upit)) {
                printf("%-5d %-20s %-20s %-30s %-15s\n",
                    k[i].id, k[i].ime, k[i].prezime, k[i].email, k[i].telefon);
                pron = 1;
            }
        break;
    }
    case 3: {
        char upit[80]; printf("Email: "); citajLiniju(upit, sizeof(upit));
        for (int i = 0; i < n; i++)
            if (strstr(k[i].email, upit)) {
                printf("%-5d %-20s %-20s %-30s %-15s\n",
                    k[i].id, k[i].ime, k[i].prezime, k[i].email, k[i].telefon);
                pron = 1;
            }
        break;
    }
    case 4: {
        char upit[20]; printf("Telefon: "); citajLiniju(upit, sizeof(upit));
        for (int i = 0; i < n; i++)
            if (strstr(k[i].telefon, upit)) {
                printf("%-5d %-20s %-20s %-30s %-15s\n",
                    k[i].id, k[i].ime, k[i].prezime, k[i].email, k[i].telefon);
                pron = 1;
            }
        break;
    }
    default:
        printf("Nevazeci izbor.\n"); return;
    }

    if (!pron) printf("Nista pronadeno.\n");
}

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
        printf("Izbor: ");
        scanf("%d", &izbor); ocistiBuffer();

        switch (izbor) {
        case 1: dodajKupca(datoteka); break;
        case 2:
            if (k) free(k);
            k = ucitajKupce(datoteka, &n);
            printf(k ? "Ucitano %d kupaca.\n" : "Datoteka prazna ili ne postoji.\n", n);
            break;
        case 3: ispisiKupce(k, n); break;
        case 4:
            urediKupca(datoteka);
            if (k) { free(k); k = ucitajKupce(datoteka, &n); }
            break;
        case 5:
            obrisiKupca(datoteka);
            if (k) { free(k); k = ucitajKupce(datoteka, &n); }
            break;
        case 6: pretraziKupce(k, n); break;
        case 7:
            if (k) { free(k); k = NULL; n = 0; }
            return;
        default:
            printf("Nevazeci izbor.\n");
        }
    }
}