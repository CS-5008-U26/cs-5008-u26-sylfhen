// header file for car

#ifndef CAR
#define CAR 1

#include <stdlib.h>
#include "engine.h"
#include "tire.h"

typedef struct carStruct {
    engine *eng;
    tire *ti[4];
} car;

car *createCar (char *engineType, int engineCc, int tirePressure);
void freeCar (car *c);
void printCar (car *c);

#endif