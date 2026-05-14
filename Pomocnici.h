#ifndef POMOCNICI_H
#define POMOCNICI_H

#include <stdio.h>
#include <string.h>


static inline void ocistiBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}


static inline void citajLiniju(char* buf, int vel) {
    if (fgets(buf, vel, stdin))
        buf[strcspn(buf, "\n")] = '\0';
    else
        buf[0] = '\0';
}

#endif 