/* engine.c - definition for a car's engine
*/

#include <stdio.h>
#include <stdlib.h>
#include "engine.h"

// Create an engine
// Caller is responsible for the string provided as engineType
engine *createEngine (char *engineType, int cc) {
    engine *e = malloc(sizeof(engine));
    // check for NULL
    e->engineType = engineType;
    e->cc = cc;
    return (e);
}

// Free an engine
void freeEngine (engine *e) {
    free (e);
}

// Print an engine
void printEngine (engine *e) {
    printf ("Engine type %s, volume %d\n", e->engineType, e->cc);
}