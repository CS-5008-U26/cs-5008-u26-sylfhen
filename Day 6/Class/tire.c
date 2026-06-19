/* tire.c - functions for tire structs
*/

#include <stdio.h>
#include <stdlib.h>
#include "tire.h"

tire *createTire (int pressure) {
    tire *t = malloc(sizeof(tire));
    // check for NULL
    t->pressure = pressure;
    return (t);
}

void freeTire (tire *t) {
    free (t);
}

void printTire (tire *t) {
    printf ("Pressure %d\n", t->pressure);
}