/*
 * citylist.c
 *
 * Reads a user-specified number of cities from uscities.csv into a
 * generic (void *) vector, then prints each city's name, state,
 * population, latitude, and longitude.
 */

/* Standard Library Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/* ------------------------------------------------------------------ */
/* City struct                                                        */
/* ------------------------------------------------------------------ */

/*
 * Struct to store city record fields.
 * name, stateID, countyFips, latitude, longitude, and population.
 * name/stateID are heap-allocated so we're not guessing at a fixed
 * buffer size for city/state names of unknown length.
 */
typedef struct city {
    char   *name;
    char   *stateID;
    int     countyFips;
    double  latitude;
    double  longitude;
    long    population;
} city;


/*
 * Strip trailing newline (and carriage return) from str.
 */
void killNewline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n')
        str[--len] = '\0';
    if (len > 0 && str[len - 1] == '\r')
        str[--len] = '\0';
}

/*
 * Remove surrounding double-quotes from a field string, in place.
 */
static void stripEnclosingQuotes(char *field) {
    size_t len = strlen(field);
    if (len >= 2 && field[0] == '"' && field[len - 1] == '"') {
        memmove(field, field + 1, len - 2);
        field[len - 2] = '\0';
    }
}

/*
 * Extract the next separator-delimited field from 'start' into 'out'.
 * Tracks whether we are inside quotes so that commas within a quoted
 * field are not treated as separators.
 */
static char *getNextField(char *start, char separator, char *out) {
    if (*start == '\0') {
        out[0] = '\0';
        return start;
    }

    int   inQuotes = 0;
    char *cursor   = start;
    char *write    = out;

    while (*cursor != '\0') {
        if (*cursor == '"') {
            inQuotes = !inQuotes;
        } else if (*cursor == separator && !inQuotes) {
            break;
        }
        *write++ = *cursor++;
    }
    *write = '\0';
    stripEnclosingQuotes(out);

    if (*cursor == separator)
        return cursor + 1;

    return cursor;
}

/*
 * Allocate a heap copy of src.
 */
static char *copyString(const char *src) {
    size_t len  = strlen(src);
    char  *copy = malloc(len + 1);
    if (copy == NULL)
        return NULL;
    strcpy(copy, src);
    return copy;
}

/*
 * Parse a CSV line into a heap-allocated city struct.
 * Column order in uscities.csv:
 */
city *stringToCity(char *line) {
    if (line == NULL)
        return NULL;

    city *c = malloc(sizeof(city));
    if (c == NULL)
        return NULL;

    /* field is large enough to hold any single CSV field from this file */
    char  field[1000];
    char *cursor = line;

    cursor = getNextField(cursor, ',', field); /* city (raw, possibly non-ASCII) */

    cursor  = getNextField(cursor, ',', field); /* city_ascii -> Name */
    c->name = copyString(field);

    cursor     = getNextField(cursor, ',', field); /* state_id -> State */
    c->stateID = copyString(field);

    cursor = getNextField(cursor, ',', field); /* state_name (skip) */

    cursor        = getNextField(cursor, ',', field); /* county_fips */
    c->countyFips = atoi(field);

    cursor = getNextField(cursor, ',', field); /* county_name (skip) */

    cursor       = getNextField(cursor, ',', field); /* lat */
    c->latitude  = atof(field);

    cursor       = getNextField(cursor, ',', field); /* lng */
    c->longitude = atof(field);

    cursor        = getNextField(cursor, ',', field); /* population */
    c->population = atol(field);

    if (c->name == NULL || c->stateID == NULL) {
        free(c->name);
        free(c->stateID);
        free(c);
        return NULL;
    }

    return c;
}

/*
 * Print city: For example, "Chicago IL, population 8489066, at (41.8375, -87.6866)"
 */
void printCity(city *c) {
    if (c == NULL)
        return;
    printf("%s %s, population %ld, at (%.4f, %.4f)\n",
           c->name, c->stateID, c->population, c->latitude, c->longitude);
}

/*
 * Free a city and its heap-allocated fields.
 */
void freeCity(city *c) {
    if (c == NULL)
        return;
    free(c->name);
    free(c->stateID);
    free(c);
}

/* ------------------------------------------------------------------ */
/* Generic vector (Vector3 style: stores void * elements)             */
/* ------------------------------------------------------------------ */

typedef struct {
    void **data;   /* array of pointers                  */
    int    used;   /* number of elements currently used  */
    int    size;   /* capacity of the data array          */
} vector3;

/*
 * Initialise a vector with a small starting capacity.
 */
void vectorInit(vector3 *v) {
    v->size = 4;                                  /* small starting size */
    v->used = 0;
    v->data = malloc(v->size * sizeof(void *));    /* buffer sized for pointers, not ints */
    if (v->data == NULL) {
        printf("Error: unable to allocate memory for vector\n");
        exit(1);
    }
}

/*
 * Insert a void * pointer at the end of the vector, doubling the
 * backing array whenever it fills up.
 */
void insertLast(vector3 *v, void *x) {
    if (v->used == v->size) {                 /* data array is all used   */
        int    newSize = v->size * 2;
        void **newData = malloc(newSize * sizeof(void *)); /* buffer sized for pointers */
        if (newData == NULL) {
            printf("Error: unable to allocate memory for vector\n");
            exit(1);
        }
        /* copy data from 0 to v->size-1 from v->data to newData */
        for (int i = 0; i < v->used; i++) {
            newData[i] = v->data[i];
        }
        v->size = newSize;
        free(v->data);
        v->data = newData;
    }
    v->data[v->used] = x;   /* v[0] to v[v->used-1] are full of data */
    v->used++;
}

/*
 * Free the vector's backing array only (elements are freed separately,
 * since the vector doesn't know how to free arbitrary void * payloads).
 */
void vectorFree(vector3 *v) {
    free(v->data);
    v->data = NULL;
    v->used = 0;
    v->size = 0;
}

/* ------------------------------------------------------------------ */
/* File I/O                                                            */
/* ------------------------------------------------------------------ */

/*
 * Read up to n cities from filename into the given vector.
 * Returns the number of cities actually read (may be less than n if
 * the file runs out of lines first), or -1 if the file can't be opened.
 */
int readCityVector(char *filename, int n, vector3 *v) {
    FILE *file = fopen(filename, "r");
    if (file == NULL)
        return -1;

    char  *buffer   = NULL;
    size_t capacity = 0;

    /* Throw away the first line (column headers) */
    if (getline(&buffer, &capacity, file) == -1) {
        fclose(file);
        free(buffer);
        return 0;
    }

    int count = 0;
    while (count < n && getline(&buffer, &capacity, file) != -1) {
        killNewline(buffer);

        city *c = stringToCity(buffer);
        if (c == NULL)
            continue;

        insertLast(v, c);
        count++;
    }

    fclose(file);
    free(buffer);
    return count;
}

/* ------------------------------------------------------------------ */
/* Main program                                                       */
/* ------------------------------------------------------------------ */

int main(void) {
    /* numBuf is used to read the requested city count from the user. */
    char numBuf[32];

    printf("How many cities: ");
    if (fgets(numBuf, sizeof(numBuf), stdin) == NULL)
        return 0;
    int numCities = atoi(numBuf);

    vector3 cities;
    vectorInit(&cities);

    int found = readCityVector("../../Resources/uscities.csv", numCities, &cities);
    if (found < 0) {
        printf("Error opening uscities.csv\n");
        vectorFree(&cities);
        return 1;
    }

    /* Print each city that was read in */
    for (int i = 0; i < cities.used; i++) {
        printCity((city *) cities.data[i]);
    }

    /* Clean up: free each city struct, then free the vector's array */
    for (int i = 0; i < cities.used; i++) {
        freeCity((city *) cities.data[i]);
    }
    vectorFree(&cities);

    return 0;
}