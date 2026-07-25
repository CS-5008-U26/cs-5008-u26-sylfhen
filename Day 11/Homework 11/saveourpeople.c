/*
 * saveourpeople.c
 *
 * The programme checks among the 200 largest US cities
 * (by population), find the subset whose name lengths sum to at
 * most 200 characters that maximizes total population saved.
 *
 * Using The 0/1 Knapsack algorithm:
 *   - "weight" of a city  = number of characters in its name
 *   - "value"  of a city  = its population
 *   - "capacity" of the knapsack = 200 (characters)
 *
 */

/*Standard Library includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOP_N_CITIES   200   /* we only consider the 200 largest cities   */
#define MAX_CAPACITY   200   /* total character budget for names          */
#define MAX_NAME_LEN   100   /* generous buffer for a single city name    */

/* City struct: name, nameLen (weight), population (value) */
typedef struct City{
    char      name[MAX_NAME_LEN];
    int       nameLen;      /* strlen(name), i.e. the knapsack "weight" */
    long long population;   /* the knapsack "value"                     */
} City;


/* Strip trailing newline / carriage return from str, in place. */
static void killNewline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n')
        str[--len] = '\0';
    if (len > 0 && str[len - 1] == '\r')
        str[--len] = '\0';
}

/* Remove surrounding double-quotes from a field string, in place. */
static void stripEnclosingQuotes(char *field) {
    size_t len = strlen(field);
    if (len >= 2 && field[0] == '"' && field[len - 1] == '"') {
        memmove(field, field + 1, len - 2);
        field[len - 2] = '\0';
    }
}

/*
 * Extract the next comma-delimited field from 'start' into 'out',
 * respecting quoted fields (so commas inside quotes are not treated
 * as separators). Returns a pointer just past the field, ready for
 * the next call.
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
 * Parse one CSV line into City *out. Returns 0 on success, -1 if the
 * line is malformed (e.g. blank line, missing name) so callers can
 * skip it instead of storing a phantom city.
 */
static int parseCityLine(char *line, City *out) {
    char  field[1000];
    char *cursor = line;

    cursor = getNextField(cursor, ',', field);           /* city (raw, skip)   */

    cursor = getNextField(cursor, ',', field);           /* city_ascii -> name */
    if (field[0] == '\0')
        return -1;                                       /* blank/malformed    */
    strncpy(out->name, field, MAX_NAME_LEN - 1);
    out->name[MAX_NAME_LEN - 1] = '\0';
    out->nameLen = (int) strlen(out->name);

    cursor = getNextField(cursor, ',', field);           /* state_id (skip)    */
    cursor = getNextField(cursor, ',', field);           /* state_name (skip)  */
    cursor = getNextField(cursor, ',', field);           /* county_fips (skip) */
    cursor = getNextField(cursor, ',', field);           /* county_name (skip) */
    cursor = getNextField(cursor, ',', field);           /* lat (skip)         */
    cursor = getNextField(cursor, ',', field);           /* lng (skip)         */

    cursor = getNextField(cursor, ',', field);           /* population         */
    if (field[0] == '\0')
        return -1;
    out->population = atoll(field);

    return 0;
}

/* I/O FILE */

/*
 * Using a handful of likely locations for uscities.csv so the program
 * works whether it's run from the project root or a build subfolder.
 */
static FILE *openCityFile(void) {
    const char *candidatePaths[] = {
        "uscities.csv",
        "Resources/uscities.csv",
        "../Resources/uscities.csv",
        "../../Resources/uscities.csv"
    };
    int numCandidates = sizeof(candidatePaths) / sizeof(candidatePaths[0]);

    for (int i = 0; i < numCandidates; i++) {
        FILE *file = fopen(candidatePaths[i], "r");
        if (file != NULL)
            return file;
    }
    return NULL;
}

/*
 * Read every city out of the file into a dynamically-growing array
 * (doubling as needed), skipping the header row and any malformed
 * lines. Returns the number of cities read, and sets *outArr to the
 * heap-allocated array.
 */
static int readAllCities(FILE *file, City **outArr) {
    int    capacity = 512;
    int    count    = 0;
    City  *arr      = malloc(capacity * sizeof(City));
    if (arr == NULL) {
        fprintf(stderr, "Error: unable to allocate memory for cities\n");
        exit(1);
    }

    char  *buffer   = NULL;
    size_t bufCap   = 0;

    /* Skip header line */
    if (getline(&buffer, &bufCap, file) == -1) {
        free(buffer);
        *outArr = arr;
        return 0;
    }

    while (getline(&buffer, &bufCap, file) != -1) {
        killNewline(buffer);
        if (buffer[0] == '\0')
            continue;   /* skip blank lines */

        if (count == capacity) {
            capacity *= 2;
            City *bigger = realloc(arr, capacity * sizeof(City));
            if (bigger == NULL) {
                fprintf(stderr, "Error: unable to grow city array\n");
                free(buffer);
                free(arr);
                exit(1);
            }
            arr = bigger;
        }

        if (parseCityLine(buffer, &arr[count]) == 0) {
            count++;
        }
        /* else: malformed line, skip it rather than storing garbage */
    }

    free(buffer);
    *outArr = arr;
    return count;
}

/* qsort comparator: descending by population (largest cities first) */
static int compareByPopulationDesc(const void *a, const void *b) {
    const City *ca = (const City *) a;
    const City *cb = (const City *) b;
    if (cb->population > ca->population) return 1;
    if (cb->population < ca->population) return -1;
    return 0;
}

/*  Knapsack Dynamic Programming approach */

/* Solve the 0/1 Knapsack problem for the given cities and print the
 * total saved population and which cities were chosen.
 */
static void solveKnapsack(City cities[], int n) {
    
    static long long dp[TOP_N_CITIES + 1][MAX_CAPACITY + 1];

    for (int w = 0; w <= MAX_CAPACITY; w++)
        dp[0][w] = 0;   /* base case: no cities considered -> nothing saved */

    for (int i = 1; i <= n; i++) {
        int       weight = cities[i - 1].nameLen;
        long long val    = cities[i - 1].population;

        for (int w = 0; w <= MAX_CAPACITY; w++) {
            if (weight <= w) {
                long long withCity    = dp[i - 1][w - weight] + val;
                long long withoutCity = dp[i - 1][w];
                dp[i][w] = (withCity > withoutCity) ? withCity : withoutCity;
            } else {
                dp[i][w] = dp[i - 1][w];   /* name too long for this budget */
            }
        }
    }

    long long maxPopulation = dp[n][MAX_CAPACITY];
    printf("Total Saved Population: %lld\n", maxPopulation);
    printf("Selected Cities:\n");

    /* Backtrack through the table to recover which cities were chosen. */
    int  chosen[TOP_N_CITIES];
    int  numChosen = 0;
    int  w         = MAX_CAPACITY;

    for (int i = n; i > 0; i--) {
        if (dp[i][w] != dp[i - 1][w]) {
            /* City i-1 was included in the optimal solution */
            chosen[numChosen++] = i - 1;
            w -= cities[i - 1].nameLen;
        }
    }

    /* Print in original (largest-population-first) order for readability */
    long long totalChars = 0;
    for (int k = numChosen - 1; k >= 0; k--) {
        City *c = &cities[chosen[k]];
        totalChars += c->nameLen;
        printf("- %s (%d chars, %lld population)\n",
               c->name, c->nameLen, c->population);
    }
    printf("Total characters used: %lld / %d\n", totalChars, MAX_CAPACITY);
}

/*  Main */

int main(void) {
    FILE *file = openCityFile();
    if (file == NULL) {
        fprintf(stderr, "Error: could not open uscities.csv\n");
        return 1;
    }

    City *allCities  = NULL;
    int   totalCount = readAllCities(file, &allCities);
    fclose(file);

    if (totalCount == 0) {
        fprintf(stderr, "Error: no city data read from file\n");
        free(allCities);
        return 1;
    }

    /* Sort every city we read by population, descending, so we can
     * pick out the true 200 LARGEST cities (not just the first 200
     * rows encountered in the file). */
    qsort(allCities, totalCount, sizeof(City), compareByPopulationDesc);

    int n = (totalCount < TOP_N_CITIES) ? totalCount : TOP_N_CITIES;

    printf("Considering the %d largest cities by population.\n\n", n);

    solveKnapsack(allCities, n);

    free(allCities);
    return 0;
}