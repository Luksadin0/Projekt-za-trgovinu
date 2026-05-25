#ifndef POMOCNICI_H
#define POMOCNICI_H

#include <stdio.h>
#include <string.h>
#include <errno.h>

/* ------------------------------------------------------------------ */
/*  Inline pomocne funkcije (tocka 9)                                  */
/* ------------------------------------------------------------------ */

/* Cisti stdin buffer nakon scanf */
static inline void ocistiBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* Citanje linije s uklanjanjem newline znaka */
static inline void citajLiniju(char* buf, int vel) {
    if (!buf || vel <= 0) return;
    if (fgets(buf, vel, stdin))
        buf[strcspn(buf, "\n")] = '\0';
    else
        buf[0] = '\0';
}

/* Ispis sistemske greske (tocka 22) */
static inline void ispisiGresku(const char* kontekst) {
    fprintf(stderr, "[GRESKA] %s: %s\n", kontekst, strerror(errno));
}

/* Sigurno kopiranje stringa s null-terminatorom */
static inline void sigurnoKopiraj(char* odrediste, const char* izvor,
    size_t vel) {
    if (!odrediste || !izvor || vel == 0) return;
    strncpy(odrediste, izvor, vel - 1);
    odrediste[vel - 1] = '\0';
}

/* ------------------------------------------------------------------ */
/*  Pomocne funkcije za datoteke (tocka 21)                            */
/* ------------------------------------------------------------------ */

/*
 * Atomicno spremanje: pise u .tmp datoteku, zatim rename.
 * Vraca 1 uspjeh, 0 greska.
 */
static inline int atomicnoSpremi(const char* datoteka,
    const void* podaci,
    int         n,
    size_t      vel) {
    char temp[512];
    snprintf(temp, sizeof(temp), "%s.tmp", datoteka);

    FILE* fp = fopen(temp, "wb");
    if (!fp) { ispisiGresku("fopen (temp)"); return 0; }

    if (fwrite(&n, sizeof(int), 1, fp) != 1) {
        ispisiGresku("fwrite (count)");
        fclose(fp);
        remove(temp);
        return 0;
    }
    if (n > 0 && fwrite(podaci, vel, (size_t)n, fp) != (size_t)n) {
        ispisiGresku("fwrite (data)");
        fclose(fp);
        remove(temp);
        return 0;
    }

    fclose(fp);
    remove(datoteka);                       /* tocka 21 – remove */
    if (rename(temp, datoteka) != 0) {      /* tocka 21 – rename */
        ispisiGresku("rename");
        return 0;
    }
    return 1;
}

#endif /* POMOCNICI_H */