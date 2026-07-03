/*
 * citylist.c
 *
 * Reads a user-specified number of cities from a CSV file into a
 * generic (void *) vector, then prints each city's name, state,
 * population, latitude, and longitude.
 */

/* Standard Libraries & State/Name Lengths */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define NAME_LEN 200
#define STATE_LEN 10

/* Generic vector of void * pointers */
typedef struct {
    void **data;   /* array of pointers                */
    int used;      /* number of elements currently used */
    int size;      /* capacity of the data array         */
} vector3;

/* Initialize a vector with a small starting capacity */
void vectorInit(vector3 *v) {
    v->size = 4;                                   /* small starting size */
    v->used = 0;
    v->data = malloc(v->size * sizeof(void *));    /* buffer sized for pointers, not ints */
    if (v->data == NULL) {
        printf("Error: unable to allocate memory for vector\n");
        exit(1);
    }
}

/* Insert a void * pointer at the end of the vector, growing if needed */
void insertLast(vector3 *v, void *x) {
    if (v->used == v->size) {              /* data array is all used   */
        int newSize = v->size * 2;
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

/* Free the vector's backing array (does not free the pointed-to elements) */
void vectorFree(vector3 *v) {
    free(v->data);
    v->data = NULL;
    v->used = 0;
    v->size = 0;
}

/* City struct definition */
typedef struct {
    char name[NAME_LEN];
    char state[STATE_LEN];
    int countyFips;
    double latitude;
    double longitude;
    int population;
} City;



/* Remove the trailing newline character from the input string */
void killNewline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

/* Extract the next comma-separated field from 'start' into 'out' */
char *getNextField(char *start, char separator, char *out) {
    /* No more fields */
    if (*start == '\0') {
        out[0] = '\0';
        return start;
    }
    /* Find the position of the next separator */
    char *sepPos = strchr(start, separator);
    if (sepPos == NULL) {
        /* No separator found copy the rest of the string */
        strcpy(out, start);
        /* Strip double-quotes if present */
        int outLen = strlen(out);
        if (outLen >= 2 && out[0] == '"' && out[outLen - 1] == '"') {
            out[outLen - 1] = '\0';
            memmove(out, out + 1, outLen - 1);
        }
        return start + strlen(start);
    } else {
        /* Copy characters up to the separator */
        size_t fieldLength = sepPos - start;
        strncpy(out, start, fieldLength);
        out[fieldLength] = '\0';
        /* Strip double-quotes if present */
        int outLen = strlen(out);
        if (outLen >= 2 && out[0] == '"' && out[outLen - 1] == '"') {
            out[outLen - 1] = '\0';
            memmove(out, out + 1, outLen - 1);
        }
        return sepPos + 1;
    }
}

/* ------------------------------------------------------------------ */
/* Parse one CSV line into a City struct (columns are 1-indexed)      */
/* ------------------------------------------------------------------ */

void parseCityLine(char *line, City *c) {
    char field[1000];   /* one raw CSV field can't exceed the line buffer size */
    char *ptr = line;
    int fieldIndex = 1;

    while (*ptr != '\0') {
        ptr = getNextField(ptr, ',', field);
        switch (fieldIndex) {
            case 2: /* Name */
                strncpy(c->name, field, NAME_LEN - 1);
                c->name[NAME_LEN - 1] = '\0';
                break;
            case 3: /* State */
                strncpy(c->state, field, STATE_LEN - 1);
                c->state[STATE_LEN - 1] = '\0';
                break;
            case 5: /* County FIPS Code */
                c->countyFips = atoi(field);
                break;
            case 7: /* Latitude */
                c->latitude = atof(field);
                break;
            case 8: /* Longitude */
                c->longitude = atof(field);
                break;
            case 9: /* Population */
                c->population = atoi(field);
                break;
            default:
                break;
        }
        fieldIndex++;
    }
}

/* ------------------------------------------------------------------ */
/* Main program                                                       */
/* ------------------------------------------------------------------ */

int main(void) {
    char filename[1000];
    char buffer[1000];  /* line buffer; large enough for any CSV row we expect */
    int numCities;

    printf("Enter the filepath: ");
    fgets(filename, sizeof(filename), stdin);
    killNewline(filename);

    /* Open the specified filepath for reading */
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening the filepath: %s\n", filename);
        return 1;
    }

    /* Throw away the first line (column headers) */
    fgets(buffer, sizeof(buffer), file);

    /* Ask the user how many cities to read */
    printf("How many cities: ");
    fgets(buffer, sizeof(buffer), stdin);
    numCities = atoi(buffer);

    /* Create the generic vector that will hold City * pointers */
    vector3 cities;
    vectorInit(&cities);

    int cityCount = 0;
    while (cityCount < numCities && fgets(buffer, sizeof(buffer), file) != NULL) {
        killNewline(buffer);

        City *c = malloc(sizeof(City));
        if (c == NULL) {
            printf("Error: unable to allocate memory for city\n");
            fclose(file);
            return 1;
        }

        parseCityLine(buffer, c);
        insertLast(&cities, c);

        cityCount++;
    }

    fclose(file);

    /* Print each city that was read in */
    for (int i = 0; i < cities.used; i++) {
        City *c = (City *) cities.data[i];
        printf("%s %s, population %d, at (%.4f, %.4f)\n",
               c->name, c->state, c->population, c->latitude, c->longitude);
    }

    /* Clean up: free each City struct, then free the vector itself */
    for (int i = 0; i < cities.used; i++) {
        free(cities.data[i]);
    }
    vectorFree(&cities);

    return 0;
}