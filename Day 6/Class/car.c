// car.c - functions for car struct

#include <stdio.h>
#include <stdlib.h>
#include "engine.h"
#include "tire.h"
#include "car.h"

car *createCar (char *engineType, int engineCc, int tirePressure) {
    car *c = malloc (sizeof(car));
    // check for NULL
    c->eng = createEngine (engineType, engineCc);
    for (int i=0; i<4; i++) {
        c->ti[i] = createTire (tirePressure);
    }
    return (c);
}

void freeCar (car *c) {
    freeEngine (c->eng);
    for (int i =0; i<4; i++) {
        freeTire (c->ti[i])
    }
    free (c);
}

void printCar (car *c) {
    printEngine (c->eng);
    for (int i=0; i<4; i++) {
        printf ("Tire %d: ", i);
        printTire (c->ti[i]);
    }
}