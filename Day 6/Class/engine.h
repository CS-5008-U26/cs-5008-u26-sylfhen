// header file for engine struct

#ifndef ENGINE
#define ENGINE 1

typedef struct engineStruct {
    char *engineType;   // the type of engine, for example "V-8"
    int cc;             // the capacity of the cylinders in cc
} engine;

engine *createEngine(char *engineType, int cc);
void freeEngine (engine *e);
void printEngine (engine *e);

#endif