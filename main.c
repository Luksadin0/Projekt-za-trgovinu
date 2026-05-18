#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pomocnici.h"
#include "proizvodi.h"
#include "kupci.h"
#include "narudzbe.h"
#include "zalihe.h"

#define DAT_PROIZVODI "proizvodi.dat"
#define DAT_KUPCI     "kupci.dat"
#define DAT_NARUDZBE  "narudzbe.dat"
#define DAT_ZALIHE    "zalihe.dat"

typedef enum {
    IZBORNIK_PROIZVODI = 1,
    IZBORNIK_KUPCI = 2,
    IZBORNIK_NARUDZBE = 3,
    IZBORNIK_ZALIHE = 4,
    IZBORNIK_IZLAZ = 5
} OpcijaGlavni;

int main(void) {
    int  izbor;
    char potvrda[10];

    while (1) {
        printf("\n");
        printf("==========================================\n");
        printf("     SUSTAV UPRAVLJANJA TRGOVINOM\n");
        printf("==========================================\n");
        printf("  1. Upravljanje proizvodima\n");
        printf("  2. Upravljanje kupcima\n");
        printf("  3. Upravljanje narudzbama\n");
        printf("  4. Upravljanje zalihama\n");
        printf("  5. Izlaz\n");
        printf("==========================================\n");
        printf("Izbor: ");
        scanf("%d", &izbor);
        ocistiBuffer();



        switch ((OpcijaGlavni)izbor) {
        case IZBORNIK_PROIZVODI:
            izbornikProizvodi(DAT_PROIZVODI);
            break;
        case IZBORNIK_KUPCI:
            izbornikKupci(DAT_KUPCI);
            break;
        case IZBORNIK_NARUDZBE:
            izbornikNarudzbe(DAT_NARUDZBE, DAT_KUPCI, DAT_PROIZVODI);
            break;
        case IZBORNIK_ZALIHE:
            izbornikZalihe(DAT_ZALIHE);
            break;
        case IZBORNIK_IZLAZ:
            printf("Jeste li sigurni da zelite zavrsiti? (da/ne): ");
            citajLiniju(potvrda, sizeof(potvrda));
            if (strcmp(potvrda, "da") == 0) {
                printf("\nDovidjenja!\n");
                return 0;
            }
            break;
        default:
            printf("Nevazeci izbor! Unesite broj od 1 do 5.\n");
        }
    }
    return 0;
}