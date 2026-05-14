#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "narudzbe.h"
#include "kupci.h"
#include "proizvodi.h"
#include "pomocnici.h"


static int sljedeciID(NARUDZBA* nar, int n) {
    int maks = 0;
    for (int i = 0; i < n; i++)
        if (nar[i].id > maks) maks = nar[i].id;
    return maks + 1;
}

static void danasnjiDatum(char* buf, int vel) {
    time_t t = time(NULL);
    struct tm* tm_info = localtime(&t);
    strftime(buf, vel, "%d.%m.%Y", tm_info);
}


NARUDZBA* ucitajNarudzbe(const char* datoteka, int* n) {
    FILE* fp = fopen(datoteka, "rb");
    if (!fp) { *n = 0; return NULL; }

    if (fread(n, sizeof(int), 1, fp) != 1 || *n <= 0) {
        fclose(fp); *n = 0; return NULL;
    }

    NARUDZBA* nar = malloc(*n * sizeof(NARUDZBA));
    if (!nar) { fclose(fp); *n = 0; return NULL; }

    fread(nar, sizeof(NARUDZBA), *n, fp);
    fclose(fp);
    return nar;
}

int spremiNarudzbe(const char* datoteka, NARUDZBA* nar, int n) {
    FILE* fp = fopen(datoteka, "wb");
    if (!fp) return 0;
    fwrite(&n, sizeof(int), 1, fp);
    if (n > 0) fwrite(nar, sizeof(NARUDZBA), n, fp);
    fclose(fp);
    return 1;
}

void ispisiNarudzbe(NARUDZBA* nar, int n) {
    if (!nar || n == 0) {
        printf("Nema ucitanih narudzbi. Koristite opciju 'Ucitaj narudzbe'.\n");
        return;
    }
    printf("\n%-5s %-8s %-10s %-8s %12s %-12s %-14s\n",
        "ID", "KupacID", "ProizvodID", "Kolicina", "Ukupno(kn)", "Datum", "Status");
    printf("%-5s %-8s %-10s %-8s %12s %-12s %-14s\n",
        "-----", "--------", "----------", "--------",
        "------------", "------------", "--------------");
    for (int i = 0; i < n; i++)
        printf("%-5d %-8d %-10d %-8d %10.2f kn %-12s %-14s\n",
            nar[i].id, nar[i].kupac_id, nar[i].proizvod_id,
            nar[i].kolicina, nar[i].ukupna_cijena,
            nar[i].datum, nar[i].status);
    printf("Ukupno: %d narudzb(a)\n", n);
}

void dodajNarudzbu(const char* dat_nar,
    const char* dat_kupci,
    const char* dat_proizvodi) {
    /* Ucitaj kupce i prikazi */
    int nk = 0;
    KUPAC* kupci = ucitajKupce(dat_kupci, &nk);
    if (!kupci || nk == 0) {
        printf("Nema kupaca u sustavu. Prvo dodajte kupca.\n");
        if (kupci) free(kupci);
        return;
    }
    printf("\n--- Dostupni kupci ---\n");
    ispisiKupce(kupci, nk);

    /* Ucitaj proizvode i prikazi */
    int np = 0;
    PROIZVOD* proizvodi = ucitajProizvode(dat_proizvodi, &np);
    if (!proizvodi || np == 0) {
        printf("Nema proizvoda u sustavu. Prvo dodajte proizvod.\n");
        free(kupci);
        if (proizvodi) free(proizvodi);
        return;
    }
    printf("\n--- Dostupni proizvodi ---\n");
    ispisiProizvode(proizvodi, np);

    /* Unos podataka narudzbe */
    NARUDZBA novi;
    int n = 0;
    NARUDZBA* nar = ucitajNarudzbe(dat_nar, &n);
    novi.id = sljedeciID(nar, n);

    /* Provjeri kupca */
    int kupacID;
    printf("\nID kupca: ");
    scanf("%d", &kupacID); ocistiBuffer();
    int kidx = -1;
    for (int i = 0; i < nk; i++)
        if (kupci[i].id == kupacID) { kidx = i; break; }
    if (kidx == -1) {
        printf("Kupac s ID-om %d ne postoji.\n", kupacID);
        free(kupci); free(proizvodi); if (nar) free(nar); return;
    }
    novi.kupac_id = kupacID;

    /* Provjeri proizvod */
    int proizvID;
    printf("ID proizvoda: ");
    scanf("%d", &proizvID); ocistiBuffer();
    int pidx = -1;
    for (int i = 0; i < np; i++)
        if (proizvodi[i].id == proizvID) { pidx = i; break; }
    if (pidx == -1) {
        printf("Proizvod s ID-om %d ne postoji.\n", proizvID);
        free(kupci); free(proizvodi); if (nar) free(nar); return;
    }
    novi.proizvod_id = proizvID;

    /* Kolicina i cijena */
    printf("Kolicina: ");
    scanf("%d", &novi.kolicina); ocistiBuffer();
    if (novi.kolicina <= 0) {
        printf("Kolicina mora biti pozitivna.\n");
        free(kupci); free(proizvodi); if (nar) free(nar); return;
    }
    novi.ukupna_cijena = proizvodi[pidx].cijena * novi.kolicina;

    /* Datum i status */
    danasnjiDatum(novi.datum, sizeof(novi.datum));
    strncpy(novi.status, "Na cekanju", sizeof(novi.status) - 1);
    novi.status[sizeof(novi.status) - 1] = '\0';

    /* Prikazi pregled narudzbe */
    printf("\n--- Pregled narudzbe ---\n");
    printf("Kupac    : %s %s\n", kupci[kidx].ime, kupci[kidx].prezime);
    printf("Proizvod : %s\n", proizvodi[pidx].ime);
    printf("Kolicina : %d kom\n", novi.kolicina);
    printf("Cijena/kom: %.2f kn\n", proizvodi[pidx].cijena);
    printf("UKUPNO   : %.2f kn\n", novi.ukupna_cijena);
    printf("Datum    : %s\n", novi.datum);
    printf("Status   : %s\n", novi.status);

    char potvrda[10];
    printf("Pohrani narudzbu? (da/ne): ");
    citajLiniju(potvrda, sizeof(potvrda));

    if (strcmp(potvrda, "da") != 0) {
        printf("Narudzba odustana.\n");
        free(kupci); free(proizvodi); if (nar) free(nar); return;
    }

    /* Proširi niz i spremi */
    NARUDZBA* tmp = realloc(nar, (n + 1) * sizeof(NARUDZBA));
    if (!tmp) {
        printf("Greska pri alokaciji.\n");
        free(kupci); free(proizvodi); if (nar) free(nar); return;
    }
    nar = tmp;
    nar[n++] = novi;

    if (spremiNarudzbe(dat_nar, nar, n))
        printf("Narudzba uspjesno pohranjena! (ID: %d, Ukupno: %.2f kn)\n",
            novi.id, novi.ukupna_cijena);
    else
        printf("Greska pri spremanju.\n");

    free(kupci); free(proizvodi); free(nar);
}

void urediNarudzbu(const char* datoteka) {
    int       n = 0;
    NARUDZBA* nar = ucitajNarudzbe(datoteka, &n);
    if (!nar) { printf("Nema podataka za uredivanje.\n"); return; }

    ispisiNarudzbe(nar, n);

    int id;
    printf("\nID narudzbe za uredivanje: ");
    scanf("%d", &id); ocistiBuffer();

    int idx = -1;
    for (int i = 0; i < n; i++)
        if (nar[i].id == id) { idx = i; break; }

    if (idx == -1) {
        printf("Narudzba s ID-om %d nije pronadena.\n", id);
        free(nar); return;
    }

    /* Dozvoljeno uredivanje: status i kolicina */
    printf("\nTrenutni status [%s]\n", nar[idx].status);
    printf("  1. Na cekanju\n  2. Isporuceno\n  3. Otkazano\n");
    printf("  0. Zadrzati trenutni\n");
    printf("Izbor: ");
    int s; scanf("%d", &s); ocistiBuffer();
    switch (s) {
    case 1: strncpy(nar[idx].status, "Na cekanju", sizeof(nar[idx].status) - 1); break;
    case 2: strncpy(nar[idx].status, "Isporuceno", sizeof(nar[idx].status) - 1); break;
    case 3: strncpy(nar[idx].status, "Otkazano", sizeof(nar[idx].status) - 1); break;
    default: break;
    }

    if (spremiNarudzbe(datoteka, nar, n))
        printf("Narudzba ID %d uspjesno uredena.\n", id);
    else
        printf("Greska pri spremanju.\n");
    free(nar);
}

void obrisiNarudzbu(const char* datoteka) {
    int       n = 0;
    NARUDZBA* nar = ucitajNarudzbe(datoteka, &n);
    if (!nar) { printf("Nema podataka za brisanje.\n"); return; }

    ispisiNarudzbe(nar, n);

    int id;
    printf("\nID narudzbe za brisanje: ");
    scanf("%d", &id); ocistiBuffer();

    int idx = -1;
    for (int i = 0; i < n; i++)
        if (nar[i].id == id) { idx = i; break; }

    if (idx == -1) {
        printf("Narudzba s ID-om %d nije pronadena.\n", id);
        free(nar); return;
    }

    char potvrda[10];
    printf("Brisanje narudzbe #%d (%.2f kn). Potvrdite (da/ne): ",
        nar[idx].id, nar[idx].ukupna_cijena);
    citajLiniju(potvrda, sizeof(potvrda));

    if (strcmp(potvrda, "da") != 0) {
        printf("Brisanje odustano.\n");
        free(nar); return;
    }

    for (int i = idx; i < n - 1; i++)
        nar[i] = nar[i + 1];
    n--;

    if (spremiNarudzbe(datoteka, nar, n))
        printf("Narudzba uspjesno obrisana.\n");
    else
        printf("Greska pri spremanju.\n");
    free(nar);
}

void pretraziNarudzbe(NARUDZBA* nar, int n) {
    if (!nar || n == 0) {
        printf("Nema ucitanih podataka. Koristite opciju 'Ucitaj narudzbe'.\n");
        return;
    }

    int opcija, pron = 0;
    printf("\nPretraga po:\n");
    printf("  1. ID narudzbe\n  2. ID kupca\n  3. ID proizvoda\n  4. Statusu\n");
    printf("Izbor: ");
    scanf("%d", &opcija); ocistiBuffer();

    printf("\n%-5s %-8s %-10s %-8s %12s %-12s %-14s\n",
        "ID", "KupacID", "ProizvodID", "Kolicina", "Ukupno(kn)", "Datum", "Status");
    printf("%-5s %-8s %-10s %-8s %12s %-12s %-14s\n",
        "-----", "--------", "----------", "--------",
        "------------", "------------", "--------------");

    switch (opcija) {
    case 1: {
        int id; printf("ID narudzbe: "); scanf("%d", &id); ocistiBuffer();
        for (int i = 0; i < n; i++)
            if (nar[i].id == id) {
                printf("%-5d %-8d %-10d %-8d %10.2f kn %-12s %-14s\n",
                    nar[i].id, nar[i].kupac_id, nar[i].proizvod_id,
                    nar[i].kolicina, nar[i].ukupna_cijena,
                    nar[i].datum, nar[i].status);
                pron = 1; break;
            }
        break;
    }
    case 2: {
        int id; printf("ID kupca: "); scanf("%d", &id); ocistiBuffer();
        for (int i = 0; i < n; i++)
            if (nar[i].kupac_id == id) {
                printf("%-5d %-8d %-10d %-8d %10.2f kn %-12s %-14s\n",
                    nar[i].id, nar[i].kupac_id, nar[i].proizvod_id,
                    nar[i].kolicina, nar[i].ukupna_cijena,
                    nar[i].datum, nar[i].status);
                pron = 1;
            }
        break;
    }
    case 3: {
        int id; printf("ID proizvoda: "); scanf("%d", &id); ocistiBuffer();
        for (int i = 0; i < n; i++)
            if (nar[i].proizvod_id == id) {
                printf("%-5d %-8d %-10d %-8d %10.2f kn %-12s %-14s\n",
                    nar[i].id, nar[i].kupac_id, nar[i].proizvod_id,
                    nar[i].kolicina, nar[i].ukupna_cijena,
                    nar[i].datum, nar[i].status);
                pron = 1;
            }
        break;
    }
    case 4: {
        char upit[20]; printf("Status: "); citajLiniju(upit, sizeof(upit));
        for (int i = 0; i < n; i++)
            if (strstr(nar[i].status, upit)) {
                printf("%-5d %-8d %-10d %-8d %10.2f kn %-12s %-14s\n",
                    nar[i].id, nar[i].kupac_id, nar[i].proizvod_id,
                    nar[i].kolicina, nar[i].ukupna_cijena,
                    nar[i].datum, nar[i].status);
                pron = 1;
            }
        break;
    }
    default:
        printf("Nevazeci izbor.\n"); return;
    }

    if (!pron) printf("Nista pronadeno.\n");
}

void izbornikNarudzbe(const char* dat_nar,
    const char* dat_kupci,
    const char* dat_proizvodi) {
    int       izbor;
    NARUDZBA* nar = NULL;
    int       n = 0;

    while (1) {
        printf("\n--- UPRAVLJANJE NARUDZB AMA ---\n");
        printf("  1. Dodaj novu narudzbu\n");
        printf("  2. Ucitaj narudzbe iz datoteke\n");
        printf("  3. Ispisi sve narudzbe\n");
        printf("  4. Uredi status narudzbe\n");
        printf("  5. Obrisi narudzbu\n");
        printf("  6. Pretrazi narudzbe\n");
        printf("  7. Povratak\n");
        printf("Izbor: ");
        scanf("%d", &izbor); ocistiBuffer();

        switch (izbor) {
        case 1: dodajNarudzbu(dat_nar, dat_kupci, dat_proizvodi); break;
        case 2:
            if (nar) free(nar);
            nar = ucitajNarudzbe(dat_nar, &n);
            printf(nar ? "Ucitano %d narudzbi.\n" : "Datoteka prazna ili ne postoji.\n", n);
            break;
        case 3: ispisiNarudzbe(nar, n); break;
        case 4:
            urediNarudzbu(dat_nar);
            if (nar) { free(nar); nar = ucitajNarudzbe(dat_nar, &n); }
            break;
        case 5:
            obrisiNarudzbu(dat_nar);
            if (nar) { free(nar); nar = ucitajNarudzbe(dat_nar, &n); }
            break;
        case 6: pretraziNarudzbe(nar, n); break;
        case 7:
            if (nar) { free(nar); nar = NULL; n = 0; }
            return;
        default:
            printf("Nevazeci izbor.\n");
        }
    }
}