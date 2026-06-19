// header file for struct tire

#ifndef TIRE
#define TIRE 1

typedef struct tireStruct {
    int pressure;
} tire;

tire *createTire (int pressure);
void freeTire (tire *t);
void printTire (tire *t);

#endif