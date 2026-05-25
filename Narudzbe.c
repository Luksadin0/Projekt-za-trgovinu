#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "narudzbe.h"
#include "kupci.h"
#include "proizvodi.h"
#include "pomocnici.h"

/* ------------------------------------------------------------------ */
/*  Static pomocne funkcije (tocka 6)                                  */
/* ------------------------------------------------------------------ */

static int sljedeciID(NARUDZBA* nar, int n) {
    int maks = 0;
    for (int i = 0; i < n; i++)
        if (nar[i].id > maks) maks = nar[i].id;
    return maks + 1;
}

static void danasnjiDatum(char* buf, int vel) {
    if (!buf || vel <= 0) return;
    time_t     t = time(NULL);
    struct tm* tmInfo = localtime(&t);
    strftime(buf, (size_t)vel, "%d.%m.%Y", tmInfo);
}

/* ------------------------------------------------------------------ */
/*  Komparatori za qsort i bsearch (tocke 23, 24, 26)                  */
/* ------------------------------------------------------------------ */

/* Pokazivac na funkciju – sortiranje po datumu (leksikografski) */
static int usporediDatum(const void* a, const void* b) {
    return strcmp(((const NARUDZBA*)a)->datum,
        ((const NARUDZBA*)b)->datum);
}

/* Komparator za bsearch po ID-u */
static int usporediNarudzbaID(const void* kljuc, const void* elem) {
    int id = *(const int*)kljuc;
    return id - ((const NARUDZBA*)elem)->id;
}

/* ------------------------------------------------------------------ */
/*  Rekurzivni binarni search po ID-u (tocka 25)                       */
/* ------------------------------------------------------------------ */
static int binSearchNarudzbaRek(NARUDZBA* nar, int lijevo, int desno,
    int trazeniID) {
    if (lijevo > desno) return -1;
    int sredina = lijevo + (desno - lijevo) / 2;
    if (nar[sredina].id == trazeniID) return sredina;
    if (nar[sredina].id < trazeniID)
        return binSearchNarudzbaRek(nar, sredina + 1, desno, trazeniID);
    return binSearchNarudzbaRek(nar, lijevo, sredina - 1, trazeniID);
}

/* ------------------------------------------------------------------ */
/*  CRUD funkcije                                                       */
/* ------------------------------------------------------------------ */

NARUDZBA* ucitajNarudzbe(const char* datoteka, int* n) {
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

    NARUDZBA* nar = malloc((size_t)(*n) * sizeof(NARUDZBA));  /* tocka 17 */
    if (!nar) { ispisiGresku("malloc"); fclose(fp); *n = 0; return NULL; }

    if ((int)fread(nar, sizeof(NARUDZBA), (size_t)(*n), fp) != *n) {
        if (ferror(fp)) ispisiGresku("fread (data)");
        free(nar); nar = NULL; *n = 0; fclose(fp); return NULL;
    }

    fclose(fp);
    return nar;
}

int spremiNarudzbe(const char* datoteka, NARUDZBA* nar, int n) {
    if (!datoteka) return 0;
    return atomicnoSpremi(datoteka, nar, n, sizeof(NARUDZBA));  /* tocka 21 */
}

void ispisiNarudzbe(NARUDZBA* nar, int n) {
    if (!nar || n <= 0) {   /* tocka 14 */
        printf("Nema ucitanih narudzbi. Koristite opciju 'Ucitaj narudzbe'.\n");
        return;
    }
    printf("\n%-5s %-8s %-10s %-8s %12s %-12s %-14s\n",
        "ID", "KupacID", "ProizvodID", "Kolicina",
        "Ukupno(EUR)", "Datum", "Status");
    printf("%-5s %-8s %-10s %-8s %12s %-12s %-14s\n",
        "-----", "--------", "----------", "--------",
        "------------", "------------", "--------------");
    for (int i = 0; i < n; i++)
        printf("%-5d %-8d %-10d %-8d %10.2f EUR %-12s %-14s\n",
            nar[i].id, nar[i].kupac_id, nar[i].proizvod_id,
            nar[i].kolicina, nar[i].ukupna_cijena,
            nar[i].datum, nar[i].status);
    printf("Ukupno: %d narudzb(a)\n", n);
}

void dodajNarudzbu(const char* dat_nar,
    const char* dat_kupci,
    const char* dat_proizvodi) {
    if (!dat_nar || !dat_kupci || !dat_proizvodi) return;

    int    nk = 0;
    KUPAC* kupci = ucitajKupce(dat_kupci, &nk);
    if (!kupci || nk == 0) {
        printf("Nema kupaca u sustavu. Prvo dodajte kupca.\n");
        if (kupci) { free(kupci); kupci = NULL; }
        return;
    }
    printf("\n--- Dostupni kupci ---\n");
    ispisiKupce(kupci, nk);

    int       np = 0;
    PROIZVOD* proizvodi = ucitajProizvode(dat_proizvodi, &np);
    if (!proizvodi || np == 0) {
        printf("Nema proizvoda u sustavu. Prvo dodajte proizvod.\n");
        free(kupci); kupci = NULL;
        if (proizvodi) { free(proizvodi); proizvodi = NULL; }
        return;
    }
    printf("\n--- Dostupni proizvodi ---\n");
    ispisiProizvode(proizvodi, np);

    int       n = 0;
    NARUDZBA* nar = ucitajNarudzbe(dat_nar, &n);

    NARUDZBA novi;
    memset(&novi, 0, sizeof(NARUDZBA));
    novi.id = sljedeciID(nar, n);

    /* Provjera kupca */
    int kupacID;
    printf("\nID kupca: ");
    scanf("%d", &kupacID); ocistiBuffer();

    int kidx = -1;
    for (int i = 0; i < nk; i++)
        if (kupci[i].id == kupacID) { kidx = i; break; }
    if (kidx == -1) {
        printf("Kupac s ID-om %d ne postoji.\n", kupacID);
        free(kupci); kupci = NULL;
        free(proizvodi); proizvodi = NULL;
        if (nar) { free(nar); nar = NULL; }
        return;
    }
    novi.kupac_id = kupacID;

    /* Provjera proizvoda */
    int proizvID;
    printf("ID proizvoda: ");
    scanf("%d", &proizvID); ocistiBuffer();

    int pidx = -1;
    for (int i = 0; i < np; i++)
        if (proizvodi[i].id == proizvID) { pidx = i; break; }
    if (pidx == -1) {
        printf("Proizvod s ID-om %d ne postoji.\n", proizvID);
        free(kupci); kupci = NULL;
        free(proizvodi); proizvodi = NULL;
        if (nar) { free(nar); nar = NULL; }
        return;
    }
    novi.proizvod_id = proizvID;

    /* Kolicina */
    printf("Kolicina: ");
    scanf("%d", &novi.kolicina); ocistiBuffer();
    if (novi.kolicina <= 0) {
        printf("Kolicina mora biti pozitivna.\n");
        free(kupci); kupci = NULL;
        free(proizvodi); proizvodi = NULL;
        if (nar) { free(nar); nar = NULL; }
        return;
    }
    novi.ukupna_cijena = proizvodi[pidx].cijena * (float)novi.kolicina;

    danasnjiDatum(novi.datum, sizeof(novi.datum));
    sigurnoKopiraj(novi.status, "Na cekanju", sizeof(novi.status));

    /* Pregled i potvrda */
    printf("\n--- Pregled narudzbe ---\n");
    printf("Kupac     : %s %s\n", kupci[kidx].ime, kupci[kidx].prezime);
    printf("Proizvod  : %s\n", proizvodi[pidx].ime);
    printf("Kolicina  : %d kom\n", novi.kolicina);
    printf("Cijena/kom: %.2f EUR\n", proizvodi[pidx].cijena);
    printf("UKUPNO    : %.2f EUR\n", novi.ukupna_cijena);
    printf("Datum     : %s\n", novi.datum);
    printf("Status    : %s\n", novi.status);

    char potvrda[10];
    printf("Pohrani narudzbu? (da/ne): ");
    citajLiniju(potvrda, sizeof(potvrda));

    if (strcmp(potvrda, "da") != 0) {
        printf("Narudzba odustana.\n");
        free(kupci); kupci = NULL;
        free(proizvodi); proizvodi = NULL;
        if (nar) { free(nar); nar = NULL; }
        return;
    }

    NARUDZBA* tmp = realloc(nar, (size_t)(n + 1) * sizeof(NARUDZBA));
    if (!tmp) {
        ispisiGresku("realloc");
        free(kupci); kupci = NULL;
        free(proizvodi); proizvodi = NULL;
        if (nar) { free(nar); nar = NULL; }
        return;
    }
    nar = tmp;
    nar[n++] = novi;

    if (spremiNarudzbe(dat_nar, nar, n))
        printf("Narudzba uspjesno pohranjena! (ID: %d, Ukupno: %.2f EUR)\n",
            novi.id, novi.ukupna_cijena);
    else
        printf("Greska pri spremanju.\n");

    free(kupci);   kupci = NULL;
    free(proizvodi); proizvodi = NULL;
    free(nar);     nar = NULL;
}

void urediNarudzbu(const char* datoteka) {
    if (!datoteka) return;

    int       n = 0;
    NARUDZBA* nar = ucitajNarudzbe(datoteka, &n);
    if (!nar) { printf("Nema podataka za uredivanje.\n"); return; }

    ispisiNarudzbe(nar, n);

    int id;
    printf("\nID narudzbe za uredivanje: ");
    scanf("%d", &id); ocistiBuffer();

    /* tocka 25 – rekurzivni binary search */
    int idx = binSearchNarudzbaRek(nar, 0, n - 1, id);
    if (idx == -1) {
        printf("Narudzba s ID-om %d nije pronadena.\n", id);
        free(nar); nar = NULL; return;
    }

    printf("\nTrenutni status [%s]\n", nar[idx].status);
    printf("  1. Na cekanju\n  2. Isporuceno\n  3. Otkazano\n");
    printf("  0. Zadrzati trenutni\n");
    printf("Izbor: ");
    int s; scanf("%d", &s); ocistiBuffer();

    switch ((OpcijaStatus)s) {
    case STATUS_NA_CEKANJU:
        sigurnoKopiraj(nar[idx].status, "Na cekanju", sizeof(nar[idx].status));
        break;
    case STATUS_ISPORUCENO:
        sigurnoKopiraj(nar[idx].status, "Isporuceno", sizeof(nar[idx].status));
        break;
    case STATUS_OTKAZANO:
        sigurnoKopiraj(nar[idx].status, "Otkazano", sizeof(nar[idx].status));
        break;
    case STATUS_ZADRZATI:
    default:
        break;
    }

    if (spremiNarudzbe(datoteka, nar, n))
        printf("Narudzba ID %d uspjesno uredena.\n", id);
    else
        printf("Greska pri spremanju.\n");

    free(nar); nar = NULL;
}

void obrisiNarudzbu(const char* datoteka) {
    if (!datoteka) return;

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
        free(nar); nar = NULL; return;
    }

    char potvrda[10];
    printf("Brisanje narudzbe #%d (%.2f EUR). Potvrdite (da/ne): ",
        nar[idx].id, nar[idx].ukupna_cijena);
    citajLiniju(potvrda, sizeof(potvrda));

    if (strcmp(potvrda, "da") != 0) {
        printf("Brisanje odustano.\n");
        free(nar); nar = NULL; return;
    }

    for (int i = idx; i < n - 1; i++)
        nar[i] = nar[i + 1];
    n--;
    memset(&nar[n], 0, sizeof(NARUDZBA));   /* tocka 18 */

    if (spremiNarudzbe(datoteka, nar, n))
        printf("Narudzba uspjesno obrisana.\n");
    else
        printf("Greska pri spremanju.\n");

    free(nar); nar = NULL;
}

/* ------------------------------------------------------------------ */
/*  Sortiranje i pretraga (tocke 23, 24, 26)                           */
/* ------------------------------------------------------------------ */

void sortirajNarudzbe(NARUDZBA* nar, int n) {
    if (!nar || n <= 1) return;
    /* tocka 23 – qsort, tocka 26 – pokazivac na funkciju usporediDatum */
    qsort(nar, (size_t)n, sizeof(NARUDZBA), usporediDatum);
}

void pretraziNarudzbe(NARUDZBA* nar, int n) {
    if (!nar || n <= 0) {
        printf("Nema ucitanih podataka. Koristite opciju 'Ucitaj narudzbe'.\n");
        return;
    }

    int opcija, pron = 0;
    printf("\nPretraga po:\n");
    printf("  1. ID narudzbe\n  2. ID kupca\n"
        "  3. ID proizvoda\n  4. Statusu\n");
    printf("Izbor: ");
    scanf("%d", &opcija); ocistiBuffer();

    printf("\n%-5s %-8s %-10s %-8s %12s %-12s %-14s\n",
        "ID", "KupacID", "ProizvodID", "Kolicina",
        "Ukupno(EUR)", "Datum", "Status");
    printf("%-5s %-8s %-10s %-8s %12s %-12s %-14s\n",
        "-----", "--------", "----------", "--------",
        "------------", "------------", "--------------");

    switch ((OpcijaNarudzbaPretraga)opcija) {
    case NARUDZBA_PRETRAGA_ID: {
        int id;
        printf("ID narudzbe: "); scanf("%d", &id); ocistiBuffer();
        /* tocka 24 – bsearch */
        NARUDZBA* nadjena = (NARUDZBA*)bsearch(
            &id, nar, (size_t)n, sizeof(NARUDZBA), usporediNarudzbaID);
        if (nadjena) {
            printf("%-5d %-8d %-10d %-8d %10.2f EUR %-12s %-14s\n",
                nadjena->id, nadjena->kupac_id, nadjena->proizvod_id,
                nadjena->kolicina, nadjena->ukupna_cijena,
                nadjena->datum, nadjena->status);
            pron = 1;
        }
        break;
    }
    case NARUDZBA_PRETRAGA_KUPAC_ID: {
        int id; printf("ID kupca: "); scanf("%d", &id); ocistiBuffer();
        for (int i = 0; i < n; i++)
            if (nar[i].kupac_id == id) {
                printf("%-5d %-8d %-10d %-8d %10.2f EUR %-12s %-14s\n",
                    nar[i].id, nar[i].kupac_id, nar[i].proizvod_id,
                    nar[i].kolicina, nar[i].ukupna_cijena,
                    nar[i].datum, nar[i].status);
                pron = 1;
            }
        break;
    }
    case NARUDZBA_PRETRAGA_PROIZVOD_ID: {
        int id; printf("ID proizvoda: "); scanf("%d", &id); ocistiBuffer();
        for (int i = 0; i < n; i++)
            if (nar[i].proizvod_id == id) {
                printf("%-5d %-8d %-10d %-8d %10.2f EUR %-12s %-14s\n",
                    nar[i].id, nar[i].kupac_id, nar[i].proizvod_id,
                    nar[i].kolicina, nar[i].ukupna_cijena,
                    nar[i].datum, nar[i].status);
                pron = 1;
            }
        break;
    }
    case NARUDZBA_PRETRAGA_STATUS: {
        char upit[20]; printf("Status: "); citajLiniju(upit, sizeof(upit));
        for (int i = 0; i < n; i++)
            if (strstr(nar[i].status, upit)) {
                printf("%-5d %-8d %-10d %-8d %10.2f EUR %-12s %-14s\n",
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

/* ------------------------------------------------------------------ */
/*  Izbornik (tocke 10, 11)                                            */
/* ------------------------------------------------------------------ */

void izbornikNarudzbe(const char* dat_nar,
    const char* dat_kupci,
    const char* dat_proizvodi) {
    int       izbor;
    NARUDZBA* nar = NULL;
    int       n = 0;

    while (1) {
        printf("\n--- UPRAVLJANJE NARUDZBAMA ---\n");
        printf("  1. Dodaj novu narudzbu\n");
        printf("  2. Ucitaj narudzbe iz datoteke\n");
        printf("  3. Ispisi sve narudzbe\n");
        printf("  4. Uredi status narudzbe\n");
        printf("  5. Obrisi narudzbu\n");
        printf("  6. Pretrazi narudzbe\n");
        printf("  7. Povratak\n");
        printf("  8. Sortiraj narudzbe po datumu\n");
        printf("Izbor: ");
        scanf("%d", &izbor); ocistiBuffer();

        switch ((OpcijaNarudzbe)izbor) {
        case NARUDZBA_DODAJ:
            dodajNarudzbu(dat_nar, dat_kupci, dat_proizvodi);
            break;
        case NARUDZBA_UCITAJ:
            if (nar) { free(nar); nar = NULL; }
            nar = ucitajNarudzbe(dat_nar, &n);
            printf(nar ? "Ucitano %d narudzbi.\n"
                : "Datoteka prazna ili ne postoji.\n", n);
            break;
        case NARUDZBA_ISPISI:
            ispisiNarudzbe(nar, n);
            break;
        case NARUDZBA_UREDI:
            urediNarudzbu(dat_nar);
            if (nar) { free(nar); nar = NULL; }
            nar = ucitajNarudzbe(dat_nar, &n);
            break;
        case NARUDZBA_OBRISI:
            obrisiNarudzbu(dat_nar);
            if (nar) { free(nar); nar = NULL; }
            nar = ucitajNarudzbe(dat_nar, &n);
            break;
        case NARUDZBA_PRETRAZI:
            pretraziNarudzbe(nar, n);
            break;
        case NARUDZBA_SORTIRAJ:
            sortirajNarudzbe(nar, n);
            printf("Narudzbe sortirane po datumu.\n");
            ispisiNarudzbe(nar, n);
            break;
        case NARUDZBA_POVRATAK:
            if (nar) { free(nar); nar = NULL; n = 0; }
            return;
        default:
            printf("Nevazeci izbor.\n");
        }
    }
}