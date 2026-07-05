/* 
 * citylist.c
 *
 * Reads a CSV file of US cities and stores them in a vector and two
 * binary search trees (BSTs). One BST is keyed on latitude, the other
 * on county FIPS code. The program demonstrates reading, storing,
 * searching, and freeing the data structures.
 */

/* Standard Library Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>


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

    cursor = getNextField(cursor, ',', field); /* city (raw, possibly nonASCII) */

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

/* Generic vector (Vector3 style: stores void * elements) */

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

 /* Free the vector's backing array only (elements are freed separately */
 
void vectorFree(vector3 *v) {
    free(v->data);
    v->data = NULL;
    v->used = 0;
    v->size = 0;
}

/* File I/O  */

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

/* Generic Binary Search Tree (stores void * city pointers)  */

typedef struct bstNode {
    void            *data;
    struct bstNode  *left;
    struct bstNode  *right;
} bstNode;

/*
 * Comparator type: returns <0 if a belongs before b, 0 if equal (by the
 * ordering key), >0 if a belongs after b. Each BST we build is keyed on
 * a single field of the city struct, so we write one comparator per key.
 */
typedef int (*compareFunc)(const void *a, const void *b);

/*
 * Compare two cities by latitude.
 */
int compareByLatitude(const void *a, const void *b) {
    const city *ca = (const city *) a;
    const city *cb = (const city *) b;
    if (ca->latitude < cb->latitude) 
        return -1;
    if (ca->latitude > cb->latitude) 
        return 1;
    return 0;
}

/*
 * Compare two cities by county FIPS code (as ints, not strings).
 */
int compareByFips(const void *a, const void *b) {
    const city *ca = (const city *) a;
    const city *cb = (const city *) b;
    return ca->countyFips - cb->countyFips;
}

/*
 * Insert data into the BST rooted at root, ordered by cmp.
 * If skipDuplicates is non-zero and a node with an equal key already
 * exists, the new data is silently dropped (not inserted, not freed --
 * the caller still owns it via the vector).
 */
bstNode *bstInsert(bstNode *root, void *data, compareFunc cmp, int skipDuplicates) {
    if (root == NULL) {
        bstNode *node = malloc(sizeof(bstNode));
        if (node == NULL) {
            printf("Error: unable to allocate memory for BST node\n");
            exit(1);
        }
        node->data  = data;
        node->left  = NULL;
        node->right = NULL;
        return node;
    }

    int c = cmp(data, root->data);
    if (c < 0) {
        root->left = bstInsert(root->left, data, cmp, skipDuplicates);
    } else if (c > 0) {
        root->right = bstInsert(root->right, data, cmp, skipDuplicates);
    } else {
        /* Equal key: either drop it (skipDuplicates) or keep both by
         * treating the duplicate as "less than or equal" and sending
         * it left. */
        if (!skipDuplicates) {
            root->left = bstInsert(root->left, data, cmp, skipDuplicates);
        }
        
    }
    return root;
}

/*
 * Count the number of nodes in the BST.
 */
int bstCount(bstNode *root) {
    if (root == NULL)
        return 0;
    return 1 + bstCount(root->left) + bstCount(root->right);
}

/*
 * Inorder traversal: visits nodes in ascending key order and writes
 * each city pointer into arr, advancing *idx as it goes.
 */
void bstInorderFill(bstNode *root, city **arr, int *idx) {
    if (root == NULL)
        return;
    bstInorderFill(root->left, arr, idx);
    arr[*idx] = (city *) root->data;
    (*idx)++;
    bstInorderFill(root->right, arr, idx);
}

/*
 * Free all BST nodes 
 */
void bstFree(bstNode *root) {
    if (root == NULL)
        return;
    bstFree(root->left);
    bstFree(root->right);
    free(root);
}


/*
 * Linear search for a city by exact name match. Returns its index in
 * arr, or -1 if not found.
 */
int linearSearchByName(city **arr, int n, const char *targetName) {
    for (int i = 0; i < n; i++) {
        if (strcmp(arr[i]->name, targetName) == 0) {
            return i;
        }
    }
    return -1;
}

/*
 * Binary search for a city by county FIPS code. Requires arr to be
 * sorted ascending by FIPS code. Returns its index in arr, or -1 if
 * not found.
 */
int binarySearchByFips(city **arr, int n, int targetFips) {
    int low = 0;
    int high = n - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (arr[mid]->countyFips == targetFips) {
            return mid;
        } else if (arr[mid]->countyFips < targetFips) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return -1;
}

/* Main program */

int main(void) {
    /* numBuf is used to read the requested city count from the user. */
    char numBuf[32];

    printf("How many cities: ");
    if (fgets(numBuf, sizeof(numBuf), stdin) == NULL)
        return 0;
    int numCities = atoi(numBuf);

    /* Task 1: read and print cities */

    vector3 cities;
    vectorInit(&cities);

    int found = readCityVector("../../Resources/uscities.csv", numCities, &cities);
    if (found < 0) {
        printf("Error opening uscities.csv\n");
        vectorFree(&cities);
        return 1;
    }

    for (int i = 0; i < cities.used; i++) {
        printCity((city *) cities.data[i]);
    }

    /* Task 2: BST by latitude + linear search */

    bstNode *latTree = NULL;
    for (int i = 0; i < cities.used; i++) {
        latTree = bstInsert(latTree, cities.data[i], compareByLatitude, 0);
    }

    city **byLatitude = malloc(cities.used * sizeof(city *));
    if (byLatitude == NULL) {
        printf("Error: unable to allocate memory for latitude array\n");
        return 1;
    }
    int latIdx = 0;
    bstInorderFill(latTree, byLatitude, &latIdx);

    int nyByLat = linearSearchByName(byLatitude, cities.used, "New York");
    if (nyByLat >= 0) {
        printf("By latitude, New York is index %d\n", nyByLat);
    } else {
        printf("New York was not found in the city list\n");
    }

    /* Task 3: BST by county FIPS + binary search */

    bstNode *fipsTree = NULL;
    for (int i = 0; i < cities.used; i++) {
        /* skipDuplicates = 1: ignore a city if its FIPS code is already
         * present in the tree, per the Task 3 spec. */
        fipsTree = bstInsert(fipsTree, cities.data[i], compareByFips, 1);
    }

    int fipsCount = bstCount(fipsTree);
    city **byFips = malloc(fipsCount * sizeof(city *));
    if (byFips == NULL) {
        printf("Error: unable to allocate memory for FIPS array\n");
        return 1;
    }
    int fipsIdx = 0;
    bstInorderFill(fipsTree, byFips, &fipsIdx);

    int targetFips  = 36081;
    int fipsFoundAt = binarySearchByFips(byFips, fipsCount, targetFips);
    if (fipsFoundAt >= 0) {
        printf("By FIPS code, %s is index %d\n",
               byFips[fipsFoundAt]->name, fipsFoundAt);
    } else {
        printf("FIPS code %d was not found in the city list\n", targetFips);
    }

    /* Cleanup */

    free(byLatitude);
    free(byFips);
    bstFree(latTree);
    bstFree(fipsTree);

    for (int i = 0; i < cities.used; i++) {
        freeCity((city *) cities.data[i]);
    }
    vectorFree(&cities);

    return 0;
}