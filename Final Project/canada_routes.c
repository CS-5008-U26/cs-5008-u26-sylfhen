/*
 * canada_routes.c
 *
 * FINAL PROJECT: Finding Shortest Routes Between Major Canadian Cities
 *                Using Graph Algorithms
 */

#define _POSIX_C_SOURCE 200809L  /* for strcasecmp and getline */
/*Standard Library includes*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>   /* strcasecmp */
#include <math.h>
#include <float.h>


/* Some compilers don't define M_PI in math.h, so define it here if
 * it's missing. */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
/* Maximum length of a city name, including NUL terminator. */
#define MAX_NAME_LEN  50
#define MAX_FIELD_LEN 256
#define EARTH_RADIUS  6371.0   /* km */
#define INF           DBL_MAX  /* "infinite" distance, as a double */

/* Forward declaration so Edge can point to City, and City can point
 * to Edge. */
typedef struct City City;

/* One edge in an adjacency list */
typedef struct Edge {
    City *destination;
    double distance;          /* km, from Haversine */
    struct Edge *next;
} Edge;

/* A vertex in the graph */
struct City {
    char name[MAX_NAME_LEN];
    double latitude;
    double longitude;

    Edge *connections;        /* adjacency list head */

    /* --- state used ONLY by Dijkstra and A* --- */
    double distance;          /* best known g(n): real path cost so far */
    double heuristic;         /* h(n): straight-line distance to goal   */
    double priority;          /* f(n) = g(n) + h(n), used by A*         */
    int    visited;
    City  *previous;
};

/* Cities live in a dynamically-sized array
  cityCapacity is the allocated size; cityCount is how
 * many slots are actually in use.
 */
City *cities       = NULL;
int   cityCount    = 0;
int   cityCapacity = 0;


/* Adds a new city to the cities[] array, growing it if necessary.
 * The name is copied into the City struct, so the caller can free
 * its own copy of the string after calling this. */
void addCity(const char *name, double latitude, double longitude) {
    if (cityCount >= cityCapacity) {
        int newCapacity = (cityCapacity == 0) ? 16 : cityCapacity * 2;
        City *grown = realloc(cities, (size_t)newCapacity * sizeof(City));
        if (grown == NULL) {
            printf("Error: unable to allocate memory for city \"%s\"\n", name);
            return;
        }
        cities = grown;
        cityCapacity = newCapacity;
    }

    City *c = &cities[cityCount];
    strncpy(c->name, name, MAX_NAME_LEN - 1);
    c->name[MAX_NAME_LEN - 1] = '\0';
    c->latitude    = latitude;
    c->longitude   = longitude;
    c->connections = NULL;
    c->distance    = INF;
    c->heuristic   = 0.0;
    c->priority    = INF;
    c->visited     = 0;
    c->previous    = NULL;
    cityCount++;
}
/* Finds a city by name, case-insensitive. Returns NULL if not found. */
City *findCity(const char *name) {
    for (int i = 0; i < cityCount; i++) {
        if (strcasecmp(cities[i].name, name) == 0) {
            return &cities[i];
        }
    }
    return NULL;
}

/* Index of a city pointer within the cities[] array, used by BFS/DFS
 * to index their own local bookkeeping arrays. Only valid because
 * `cities` is a single contiguous block and doesn't move once
 * buildGraph() has started building edges (see invariant above). */
int cityIndex(City *c) {
    return (int)(c - cities);
}

/* HAVERSINE FORMULA */
double degreesToRadians(double degrees) {
    return degrees * M_PI / 180.0;
}

/* Great-circle distance between two coordinates, in km. */
double haversine(double lat1, double lon1, double lat2, double lon2) {
    double dLat = degreesToRadians(lat2 - lat1);
    double dLon = degreesToRadians(lon2 - lon1);

    double a = sin(dLat / 2) * sin(dLat / 2)
             + cos(degreesToRadians(lat1)) * cos(degreesToRadians(lat2))
               * sin(dLon / 2) * sin(dLon / 2);

    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return EARTH_RADIUS * c;
}

/* GRAPH EDGES */

/* Adds a one-way edge from `from` to `to` with the given distance.
 * Used by connectCities() to add both directions of a two-way road. */
void addEdge(City *from, City *to, double distance) {
    Edge *newEdge = malloc(sizeof(Edge));
    if (newEdge == NULL) {
        printf("Error: unable to allocate memory for edge\n");
        exit(1);
    }
    newEdge->destination = to;
    newEdge->distance = distance;
    newEdge->next = from->connections;
    from->connections = newEdge;
}

/* Returns 1 if an edge from `from` to `to` already exists. Used so
 * connectCities() doesn't build duplicate roads if it's ever called
 * twice for the same pair. */
int edgeExists(City *from, City *to) {
    for (Edge *edge = from->connections; edge != NULL; edge = edge->next) {
        if (edge->destination == to) {
            return 1;
        }
    }
    return 0;
}

/* Adds a two-way road between two named cities. Distance is computed
 * automatically from their coordinates via Haversine, which also
 * guarantees the A* heuristic (straight-line distance to goal) never
 * overestimates any edge's real cost - i.e. it stays admissible.
 * Returns 1 if the edge was added, 0 if it was skipped (unknown city,
 * self-loop, or duplicate) - used by loadEdgesFromCSV() to report how
 * many rows actually became roads. */
int connectCities(const char *city1, const char *city2) {
    City *a = findCity(city1);
    City *b = findCity(city2);

    if (a == NULL || b == NULL) {
        printf("Warning: could not connect \"%s\" and \"%s\" "
               "(city not found)\n", city1, city2);
        return 0;
    }

    if (a == b) {
        printf("Warning: ignoring self-loop for \"%s\"\n", city1);
        return 0;
    }

    if (edgeExists(a, b)) {
        printf("Warning: \"%s\" <-> \"%s\" already connected, skipping duplicate\n",
               city1, city2);
        return 0;
    }

    double distance = haversine(a->latitude, a->longitude,
                                 b->latitude, b->longitude);

    addEdge(a, b, distance);
    addEdge(b, a, distance);
    return 1;
}


/* CSV LOADING */
/* Strip surrounding double-quotes from a field string, in place. */
static void stripEnclosingQuotes(char *field) {
    size_t len = strlen(field);
    if (len >= 2 && field[0] == '"' && field[len - 1] == '"') {
        memmove(field, field + 1, len - 2);
        field[len - 2] = '\0';
    }
}

/*
 * Extract the next comma-delimited field from 'start' into 'out'
 * (bounded by outSize). Tracks whether we're inside quotes so a comma
 * inside a quoted field isn't treated as a separator. Returns a
 * pointer to just past the separator (or to the terminating NUL if
 * this was the last field).
 */
static char *getNextField(char *start, char *out, size_t outSize) {
    if (*start == '\0') {
        out[0] = '\0';
        return start;
    }

    int    inQuotes = 0;
    char  *cursor    = start;
    size_t written    = 0;

    while (*cursor != '\0') {
        if (*cursor == '"') {
            inQuotes = !inQuotes;
        } else if (*cursor == ',' && !inQuotes) {
            break;
        }
        if (written + 1 < outSize) {
            out[written++] = *cursor;
        }
        cursor++;
    }
    out[written] = '\0';
    stripEnclosingQuotes(out);

    if (*cursor == ',') {
        return cursor + 1;
    }
    return cursor;
}

static void killNewline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') str[--len] = '\0';
    if (len > 0 && str[len - 1] == '\r') str[--len] = '\0';
}

/* Parses a decimal number from `s`, failing if `s` is empty or has
 * any trailing non-numeric junk, rather than silently returning 0
 * the way atof() would. */
static int parseDouble(const char *s, double *out) {
    if (s[0] == '\0') return 0;
    char *endptr;
    double value = strtod(s, &endptr);
    if (endptr == s || *endptr != '\0') return 0;
    *out = value;
    return 1;
}

/*
 * Parses one CSV data line into a name/lat/lng triple, expecting the
 * canadacities.csv column order:
 *   city, city_ascii, province_id, province_name, lat, lng, population, ...
 * (columns after lng are ignored). Returns 1 on success, 0 if the line
 * is malformed (missing name, or lat/lng aren't valid coordinates),
 * so a blank or corrupt row is skipped rather than silently becoming
 * a city with garbage coordinates.
 */
int parseCityLine(char *line, char *nameOut, double *latOut, double *lngOut) {
    if (line == NULL || line[0] == '\0') return 0;

    char  field[MAX_FIELD_LEN];
    char *cursor = line;

    cursor = getNextField(cursor, field, sizeof field); /* city (non-ASCII original, skip) */
    cursor = getNextField(cursor, field, sizeof field); /* city_ascii -> name */
    if (field[0] == '\0') return 0;
    strncpy(nameOut, field, MAX_NAME_LEN - 1);
    nameOut[MAX_NAME_LEN - 1] = '\0';

    cursor = getNextField(cursor, field, sizeof field); /* province_id (skip) */
    cursor = getNextField(cursor, field, sizeof field); /* province_name (skip) */

    cursor = getNextField(cursor, field, sizeof field); /* lat */
    if (!parseDouble(field, latOut) || *latOut < -90.0 || *latOut > 90.0) return 0;

    cursor = getNextField(cursor, field, sizeof field); /* lng */
    if (!parseDouble(field, lngOut) || *lngOut < -180.0 || *lngOut > 180.0) return 0;

    return 1;
}

/* Pass 1 of the two-pass loader: counts how many data lines in the
 * already-open file parse as valid cities */
static int countValidCityLines(FILE *fp) {
    char  *buffer   = NULL;
    size_t bufCap   = 0;
    int    count    = 0;
    char   name[MAX_NAME_LEN];
    double lat, lng;

    if (getline(&buffer, &bufCap, fp) == -1) { /* header line */
        free(buffer);
        return 0;
    }

    while (getline(&buffer, &bufCap, fp) != -1) {
        killNewline(buffer);
        if (parseCityLine(buffer, name, &lat, &lng)) {
            count++;
        }
    }

    free(buffer);
    return count;
}

/* Tries a short list of likely relative locations for the CSV, since
 * the working directory when running the program can vary depending
 * on how it's launched (IDE "Run" button vs. terminal, etc). */
static FILE *openCityCSV(void) {
    const char *candidatePaths[] = {
        "Resources/canadacities.csv",
        "../Resources/canadacities.csv",
        "../../Resources/canadacities.csv",
        "Final Project/Resources/canadacities.csv",
        "canadacities.csv"
    };
    int numCandidates = (int)(sizeof(candidatePaths) / sizeof(candidatePaths[0]));

    for (int i = 0; i < numCandidates; i++) {
        FILE *file = fopen(candidatePaths[i], "r");
        if (file != NULL) {
            return file;
        }
    }
    return NULL;
}


int loadCitiesFromCSV(void) {
    FILE *fp = openCityCSV();
    if (fp == NULL) {
        return 0;
    }

    int validCount = countValidCityLines(fp);
    if (validCount <= 0) {
        fclose(fp);
        return 0;
    }

    /* Pre-size the array exactly, so the addCity() calls below never
     * hit the realloc/growth path. */
    cities = malloc((size_t)validCount * sizeof(City));
    if (cities == NULL) {
        printf("Error: unable to allocate memory for %d cities\n", validCount);
        fclose(fp);
        exit(1);
    }
    cityCapacity = validCount;
    cityCount    = 0;

    rewind(fp);
    char  *buffer = NULL;
    size_t bufCap = 0;
    getline(&buffer, &bufCap, fp); /* skip header again */

    char   name[MAX_NAME_LEN];
    double lat, lng;
    while (cityCount < cityCapacity && getline(&buffer, &bufCap, fp) != -1) {
        killNewline(buffer);
        if (parseCityLine(buffer, name, &lat, &lng)) {
            addCity(name, lat, lng);
        }
    }

    free(buffer);
    fclose(fp);

    printf("Loaded %d cities from canadacities.csv\n", cityCount);
    return cityCount > 0;
}

/* EDGE (ROAD) CSV LOADING */

/*
 * Parses one edge CSV line, expecting the format:
 *   city1,city2
 * (a header row "city1,city2" is skipped by the caller, same as the
 * city CSV). Returns 1 on success, 0 if either field is empty.
 */
int parseEdgeLine(char *line, char *city1Out, char *city2Out) {
    if (line == NULL || line[0] == '\0') return 0;

    char  field[MAX_FIELD_LEN];
    char *cursor = line;

    cursor = getNextField(cursor, field, sizeof field); /* city1 */
    if (field[0] == '\0') return 0;
    strncpy(city1Out, field, MAX_NAME_LEN - 1);
    city1Out[MAX_NAME_LEN - 1] = '\0';

    cursor = getNextField(cursor, field, sizeof field); /* city2 */
    if (field[0] == '\0') return 0;
    strncpy(city2Out, field, MAX_NAME_LEN - 1);
    city2Out[MAX_NAME_LEN - 1] = '\0';

    return 1;
}

/* Tries a short list of likely relative locations for the edges CSV,
 * mirroring openCityCSV() above. */
static FILE *openEdgesCSV(void) {
    const char *candidatePaths[] = {
        "Resources/canada_edges.csv",
        "../Resources/canada_edges.csv",
        "../../Resources/canada_edges.csv",
        "Final Project/Resources/canada_edges.csv",
        "canada_edges.csv"
    };
    int numCandidates = (int)(sizeof(candidatePaths) / sizeof(candidatePaths[0]));

    for (int i = 0; i < numCandidates; i++) {
        FILE *file = fopen(candidatePaths[i], "r");
        if (file != NULL) {
            return file;
        }
    }
    return NULL;
}

/*
 * Loads the road network from canada_edges.csv - one pass is enough
 * here (unlike the city loader) because edges don't need to be
 * pre-allocated in a contiguous array; each one is just a separate
 * malloc'd linked-list node added via connectCities(). Returns 1 if
 * at least one edge was successfully connected, 0 if the file
 * couldn't be found/opened or contained no usable rows
 */
int loadEdgesFromCSV(void) {
    FILE *fp = openEdgesCSV();
    if (fp == NULL) {
        return 0;
    }

    char  *buffer = NULL;
    size_t bufCap = 0;

    if (getline(&buffer, &bufCap, fp) == -1) { /* header line */
        free(buffer);
        fclose(fp);
        return 0;
    }

    int    edgesConnected = 0;
    char   city1[MAX_NAME_LEN], city2[MAX_NAME_LEN];
    while (getline(&buffer, &bufCap, fp) != -1) {
        killNewline(buffer);
        if (parseEdgeLine(buffer, city1, city2)) {
            edgesConnected += connectCities(city1, city2);
        }
    }

    free(buffer);
    fclose(fp);

    printf("Loaded %d road connections from canada_edges.csv\n", edgesConnected);
    return edgesConnected > 0;
}

void freeGraph(void) {
    for (int i = 0; i < cityCount; i++) {
        Edge *edge = cities[i].connections;
        while (edge != NULL) {
            Edge *next = edge->next;
            free(edge);
            edge = next;
        }
        cities[i].connections = NULL;
    }
}


/* DISPLAY GRAPH */

void printGraph(void) {
    printf("\nCanadian City Graph\n");
    printf("====================\n\n");
    printf("(Only cities with at least one road connection are shown.)\n\n");

    for (int i = 0; i < cityCount; i++) {
        City *city = &cities[i];
        if (city->connections == NULL) {
            continue; /* skip the many CSV cities that aren't part of the road network */
        }
        printf("%s:\n", city->name);

        Edge *edge = city->connections;
        while (edge != NULL) {
            printf("   -> %-12s (%.1f km)\n",
                   edge->destination->name, edge->distance);
            edge = edge->next;
        }
        printf("\n");
    }
}


/* SHARED HELPERS */

/* Reset the fields used by Dijkstra/A* before a fresh run. (BFS/DFS
 * keep their own local state and never need this.) */
void resetWeightedState(void) {
    for (int i = 0; i < cityCount; i++) {
        cities[i].distance  = INF;
        cities[i].heuristic = 0.0;
        cities[i].priority  = INF;
        cities[i].visited   = 0;
        cities[i].previous  = NULL;
    }
}

/* Recursively print the route stored in each city's `previous` link,
 * from start to `destination`. Used by Dijkstra and A*. */
void printPath(City *destination) {
    if (destination == NULL) {
        return;
    }
    if (destination->previous != NULL) {
        printPath(destination->previous);
        printf(" -> ");
    }
    printf("%s", destination->name);
}

/***********************
       BINARY MIN-HEAP (lazy deletion)
************************/

/*
 * A small binary min-heap keyed on `priority`, storing city indices.
 * Used by both Dijkstra (priority = distance) and A* (priority =
 * distance + heuristic) O((V+E) log V).
 */
typedef struct {
    double priority;
    int    cityIdx;
} HeapEntry;

typedef struct {
    HeapEntry *data;
    int size;
    int capacity;
} MinHeap;

void heapInit(MinHeap *h, int capacityHint) {
    h->capacity = (capacityHint > 0) ? capacityHint : 16;
    h->data = malloc((size_t)h->capacity * sizeof(HeapEntry));
    if (h->data == NULL) {
        printf("Error: unable to allocate memory for heap\n");
        exit(1);
    }
    h->size = 0;
}

void heapFree(MinHeap *h) {
    free(h->data);
    h->data = NULL;
    h->size = 0;
    h->capacity = 0;
}

static void heapSwap(HeapEntry *a, HeapEntry *b) {
    HeapEntry tmp = *a;
    *a = *b;
    *b = tmp;
}

static void heapSiftUp(MinHeap *h, int idx) {
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (h->data[parent].priority <= h->data[idx].priority) {
            break;
        }
        heapSwap(&h->data[parent], &h->data[idx]);
        idx = parent;
    }
}

static void heapSiftDown(MinHeap *h, int idx) {
    while (1) {
        int left = 2 * idx + 1;
        int right = 2 * idx + 2;
        int smallest = idx;

        if (left < h->size && h->data[left].priority < h->data[smallest].priority) {
            smallest = left;
        }
        if (right < h->size && h->data[right].priority < h->data[smallest].priority) {
            smallest = right;
        }
        if (smallest == idx) {
            break;
        }
        heapSwap(&h->data[idx], &h->data[smallest]);
        idx = smallest;
    }
}

void heapPush(MinHeap *h, double priority, int cityIdx) {
    if (h->size == h->capacity) {
        int newCapacity = h->capacity * 2;
        HeapEntry *grown = realloc(h->data, (size_t)newCapacity * sizeof(HeapEntry));
        if (grown == NULL) {
            printf("Error: unable to grow heap\n");
            exit(1);
        }
        h->data = grown;
        h->capacity = newCapacity;
    }
    h->data[h->size].priority = priority;
    h->data[h->size].cityIdx  = cityIdx;
    heapSiftUp(h, h->size);
    h->size++;
}

/* Pops the minimum-priority entry into *cityIdxOut. Returns 1 on
 * success, 0 if the heap is empty. */
int heapPop(MinHeap *h, int *cityIdxOut) {
    if (h->size == 0) {
        return 0;
    }
    *cityIdxOut = h->data[0].cityIdx;
    h->size--;
    h->data[0] = h->data[h->size];
    heapSiftDown(h, 0);
    return 1;
}

/* BFS  (unweighted) */

/* Breadth-first search from start to goal. Finds the path with the
 * fewest hops (edges), ignoring distance weights. Fills `path` with
 * the city indices from start to goal (inclusive) and returns the
 * path length, or returns -1 if goal is unreachable. `*citiesExplored`
 * receives the number of cities dequeued during the search.
 *
 * `path` must be sized for at least cityCount entries by the caller. */
int bfs(City *start, City *goal, int path[], int *citiesExplored) {
    int *visited  = calloc((size_t)cityCount, sizeof(int));
    int *previous = malloc((size_t)cityCount * sizeof(int));
    int *queue    = malloc((size_t)cityCount * sizeof(int));
    if (visited == NULL || previous == NULL || queue == NULL) {
        printf("Error: unable to allocate memory for BFS\n");
        exit(1);
    }

    for (int i = 0; i < cityCount; i++) previous[i] = -1;

    int front = 0, back = 0;
    int startIdx = cityIndex(start);
    int goalIdx  = cityIndex(goal);

    visited[startIdx] = 1;
    queue[back++] = startIdx;
    *citiesExplored = 0;

    while (front < back) {
        int currentIdx = queue[front++];
        (*citiesExplored)++;
        City *current = &cities[currentIdx];

        if (currentIdx == goalIdx) {
            break;
        }

        for (Edge *edge = current->connections; edge != NULL; edge = edge->next) {
            int neighborIdx = cityIndex(edge->destination);
            if (!visited[neighborIdx]) {
                visited[neighborIdx] = 1;
                previous[neighborIdx] = currentIdx;
                queue[back++] = neighborIdx;
            }
        }
    }

    int len = -1;
    if (visited[goalIdx]) {
        /* Walk the previous[] chain backwards, then reverse it into path[] */
        int *reversed = malloc((size_t)cityCount * sizeof(int));
        if (reversed == NULL) {
            printf("Error: unable to allocate memory for BFS path\n");
            exit(1);
        }
        len = 0;
        int idx = goalIdx;
        while (idx != -1) {
            reversed[len++] = idx;
            idx = previous[idx];
        }
        for (int i = 0; i < len; i++) {
            path[i] = reversed[len - 1 - i];
        }
        free(reversed);
    }

    free(visited);
    free(previous);
    free(queue);
    return len; /* -1 if unreachable */
}

void runBFS(City *start, City *goal) {
    int *path = malloc((size_t)cityCount * sizeof(int));
    if (path == NULL) {
        printf("Error: unable to allocate memory for BFS\n");
        exit(1);
    }

    int explored;
    int hops = bfs(start, goal, path, &explored);

    printf("\n============================\n");
    printf("BFS RESULT\n");
    printf("============================\n\n");

    if (hops == -1) {
        printf("No path exists from %s to %s.\n", start->name, goal->name);
    } else {
        printf("Route (fewest hops, %d edges):\n", hops - 1);
        for (int i = 0; i < hops; i++) {
            printf("%s%s", cities[path[i]].name, (i < hops - 1) ? " -> " : "\n");
        }
    }
    printf("Cities explored: %d\n", explored);

    free(path);
}

/* DFS  (unweighted) */

/* Depth-first search from start to goal, recursive. Finds *a* path
 * (not guaranteed shortest). Returns 1 if goal is found, 0 otherwise.
 * `path` accumulates the cities visited on the current recursion
 * branch; `*pathLen` tracks how many are currently on it;
 * `*citiesExplored` counts every city visited during search.
 */
int dfsHelper(City *current, City *goal, int visited[], int path[],
              int *pathLen, int *citiesExplored) {
    int currentIdx = cityIndex(current);
    visited[currentIdx] = 1;
    path[(*pathLen)++] = currentIdx;
    (*citiesExplored)++;

    if (current == goal) {
        return 1;
    }

    for (Edge *edge = current->connections; edge != NULL; edge = edge->next) {
        int neighborIdx = cityIndex(edge->destination);
        if (!visited[neighborIdx]) {
            if (dfsHelper(edge->destination, goal, visited, path, pathLen, citiesExplored)) {
                return 1;
            }
        }
    }

    /* Dead end: back out of this city before returning to caller. */
    (*pathLen)--;
    return 0;
}

void runDFS(City *start, City *goal) {
    int *visited = calloc((size_t)cityCount, sizeof(int));
    int *path    = malloc((size_t)cityCount * sizeof(int));
    if (visited == NULL || path == NULL) {
        printf("Error: unable to allocate memory for DFS\n");
        exit(1);
    }

    int pathLen  = 0;
    int explored = 0;
    int found = dfsHelper(start, goal, visited, path, &pathLen, &explored);

    printf("\n============================\n");
    printf("DFS RESULT\n");
    printf("============================\n\n");

    if (!found) {
        printf("No path exists from %s to %s.\n", start->name, goal->name);
    } else {
        printf("A path found by DFS (not necessarily shortest):\n");
        for (int i = 0; i < pathLen; i++) {
            printf("%s%s", cities[path[i]].name, (i < pathLen - 1) ? " -> " : "\n");
        }
    }
    printf("Cities explored: %d\n", explored);

    free(visited);
    free(path);
}

/* DIJKSTRA */

/* Runs Dijkstra from start to goal using a binary min-heap
  O((V+E) log V). Returns the number of cities visited
 * (settled) by the time the goal is reached. The shortest distance
 * ends up in goal->distance, and the route can be printed with
 * printPath(goal). */
int dijkstra(City *start, City *goal) {
    resetWeightedState();
    int explored = 0;
    start->distance = 0;

    MinHeap heap;
    heapInit(&heap, cityCount);
    heapPush(&heap, 0.0, cityIndex(start));

    int idx;
    while (heapPop(&heap, &idx)) {
        City *current = &cities[idx];

        if (current->visited) {
            continue; /* stale entry from an earlier, since-improved distance */
        }
        current->visited = 1;
        explored++;

        if (current == goal) {
            break;
        }

        for (Edge *edge = current->connections; edge != NULL; edge = edge->next) {
            City *neighbor = edge->destination;
            if (neighbor->visited) {
                continue;
            }
            double newDistance = current->distance + edge->distance;
            if (newDistance < neighbor->distance) {
                neighbor->distance = newDistance;
                neighbor->previous = current;
                heapPush(&heap, newDistance, cityIndex(neighbor));
            }
        }
    }

    heapFree(&heap);
    return explored;
}

void printDijkstraResult(City *start, City *end, int explored) {
    printf("\n============================\n");
    printf("DIJKSTRA RESULTS\n");
    printf("============================\n\n");

    if (end->distance >= INF) {
        printf("No path exists from %s to %s.\n", start->name, end->name);
        return;
    }

    printf("Route:\n");
    printPath(end);
    printf("\n\nDistance: %.2f km\n", end->distance);
    printf("Cities explored: %d\n", explored);
}

/* A* Algorithm*/

double heuristic(City *current, City *goal) {
    return haversine(current->latitude, current->longitude,
                      goal->latitude, goal->longitude);
}

/* Runs A* from start to goal, guided by the straight-line-distance
 * heuristic to goal, using the same binary min-heap as Dijkstra (keyed
 * on f(n) = g(n) + h(n) instead of just g(n)). Returns the number of
 * cities visited (settled) by the time the goal is reached. The
 * shortest distance ends up in goal->distance */
int aStar(City *start, City *goal) {
    resetWeightedState();
    int explored = 0;

    start->distance  = 0;
    start->heuristic = heuristic(start, goal);
    start->priority  = start->distance + start->heuristic;

    MinHeap heap;
    heapInit(&heap, cityCount);
    heapPush(&heap, start->priority, cityIndex(start));

    int idx;
    while (heapPop(&heap, &idx)) {
        City *current = &cities[idx];

        if (current->visited) {
            continue; /* stale entry from an earlier, since-improved priority */
        }
        current->visited = 1;
        explored++;

        if (current == goal) {
            break;
        }

        for (Edge *edge = current->connections; edge != NULL; edge = edge->next) {
            City *neighbor = edge->destination;
            if (neighbor->visited) {
                continue;
            }
            double newDistance = current->distance + edge->distance;
            if (newDistance < neighbor->distance) {
                neighbor->distance  = newDistance;
                neighbor->heuristic = heuristic(neighbor, goal);
                neighbor->priority  = neighbor->distance + neighbor->heuristic;
                neighbor->previous  = current;
                heapPush(&heap, neighbor->priority, cityIndex(neighbor));
            }
        }
    }

    heapFree(&heap);
    return explored;
}

void printAStarResult(City *start, City *end, int explored) {
    printf("\n============================\n");
    printf("A* RESULTS\n");
    printf("============================\n\n");

    if (end->distance >= INF) {
        printf("No path exists from %s to %s.\n", start->name, end->name);
        return;
    }

    printf("Route:\n");
    printPath(end);
    printf("\n\nDistance: %.2f km\n", end->distance);
    printf("Cities explored: %d\n", explored);
}

/* COMPARISON SECTION */

void compareAlgorithms(double dijkstraDistance, int dijkstraNodes,
                        double aStarDistance, int aStarNodes) {
    printf("\n============================\n");
    printf("ALGORITHM COMPARISON\n");
    printf("============================\n\n");

    printf("Dijkstra distance: %.2f km\n", dijkstraDistance);
    printf("A* distance:       %.2f km\n", aStarDistance);
    printf("\n");
    printf("Dijkstra explored: %d cities\n", dijkstraNodes);
    printf("A* explored:       %d cities\n", aStarNodes);
    printf("\n");

    if (fabs(dijkstraDistance - aStarDistance) > 0.01) {
        printf("Note: the two algorithms found different-length "
               "routes; something is wrong, since both are guaranteed "
               "optimal on a graph with non-negative weights.\n");
    }

    if (aStarNodes < dijkstraNodes) {
        double improvement = ((double)(dijkstraNodes - aStarNodes) / dijkstraNodes) * 100;
        printf("A* explored %.2f%% fewer cities than Dijkstra.\n", improvement);
    } else if (aStarNodes > dijkstraNodes) {
        double difference = ((double)(aStarNodes - dijkstraNodes) / dijkstraNodes) * 100;
        printf("A* explored %.2f%% more cities than Dijkstra "
               "(can happen on small/dense graphs).\n", difference);
    } else {
        printf("Both algorithms explored the same number of cities.\n");
    }
}

/* MENU / INPUT HELPERS */

/* Prompts for a city name until a valid one is entered. */
City *promptForCity(const char *prompt) {
    char buffer[MAX_NAME_LEN];
    City *city = NULL;

    while (city == NULL) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            return NULL; /* EOF */
        }
        killNewline(buffer);
        city = findCity(buffer);
        if (city == NULL) {
            printf("Unknown city \"%s\". Try again (see menu option 1 "
                   "for the full list).\n", buffer);
        }
    }
    return city;
}

void runComparison(City *start, City *goal) {
    int dijkstraExplored = dijkstra(start, goal);
    double dijkstraDistance = goal->distance;
    printDijkstraResult(start, goal, dijkstraExplored);

    int aStarExplored = aStar(start, goal);
    double aStarDistance = goal->distance;
    printAStarResult(start, goal, aStarExplored);

    compareAlgorithms(dijkstraDistance, dijkstraExplored, aStarDistance, aStarExplored);
}

/* Runs BFS and DFS between the same two cities, prints both results,
 * then a short comparison of path length (edges) and cities explored.
 * Unlike Dijkstra vs. A* (which must always agree on distance), BFS
 * and DFS can legitimately disagree on path length - BFS is
 * guaranteed fewest hops, DFS is not - so the comparison here reports
 * that difference instead of flagging it as an error. */
void runBFSvsDFS(City *start, City *goal) {
    int *bfsPath = malloc((size_t)cityCount * sizeof(int));
    if (bfsPath == NULL) {
        printf("Error: unable to allocate memory for BFS\n");
        exit(1);
    }
    int bfsExplored;
    int bfsHops = bfs(start, goal, bfsPath, &bfsExplored);

    printf("\n============================\n");
    printf("BFS RESULT\n");
    printf("============================\n\n");
    if (bfsHops == -1) {
        printf("No path exists from %s to %s.\n", start->name, goal->name);
    } else {
        printf("Route (fewest hops, %d edges):\n", bfsHops - 1);
        for (int i = 0; i < bfsHops; i++) {
            printf("%s%s", cities[bfsPath[i]].name, (i < bfsHops - 1) ? " -> " : "\n");
        }
    }
    printf("Cities explored: %d\n", bfsExplored);
    free(bfsPath);

    int *dfsVisited = calloc((size_t)cityCount, sizeof(int));
    int *dfsPath    = malloc((size_t)cityCount * sizeof(int));
    if (dfsVisited == NULL || dfsPath == NULL) {
        printf("Error: unable to allocate memory for DFS\n");
        exit(1);
    }
    int dfsPathLen  = 0;
    int dfsExplored = 0;
    int dfsFound = dfsHelper(start, goal, dfsVisited, dfsPath, &dfsPathLen, &dfsExplored);

    printf("\n============================\n");
    printf("DFS RESULT\n");
    printf("============================\n\n");
    if (!dfsFound) {
        printf("No path exists from %s to %s.\n", start->name, goal->name);
    } else {
        printf("A path found by DFS (not necessarily shortest):\n");
        for (int i = 0; i < dfsPathLen; i++) {
            printf("%s%s", cities[dfsPath[i]].name, (i < dfsPathLen - 1) ? " -> " : "\n");
        }
    }
    printf("Cities explored: %d\n", dfsExplored);
    free(dfsVisited);
    free(dfsPath);

    printf("\n============================\n");
    printf("BFS vs. DFS COMPARISON\n");
    printf("============================\n\n");

    if (bfsHops == -1 && !dfsFound) {
        printf("Neither BFS nor DFS found a path from %s to %s.\n", start->name, goal->name);
        return;
    }
    if (bfsHops == -1 || !dfsFound) {
        printf("Note: one algorithm found a path and the other didn't; something is "
               "wrong, since both search the same connected graph.\n");
        return;
    }

    int bfsEdges = bfsHops - 1;
    int dfsEdges = dfsPathLen - 1;

    printf("BFS path length: %d edges\n", bfsEdges);
    printf("DFS path length: %d edges\n", dfsEdges);
    printf("\n");
    printf("BFS explored: %d cities\n", bfsExplored);
    printf("DFS explored: %d cities\n", dfsExplored);
    printf("\n");

    if (dfsEdges > bfsEdges) {
        printf("DFS took %d more edge(s) than the guaranteed-shortest BFS route "
               "(DFS does not guarantee the fewest hops).\n", dfsEdges - bfsEdges);
    } else {
        printf("DFS happened to find a path just as short as BFS this time "
               "(not guaranteed in general).\n");
    }
}

void printMenu(void) {
    printf("\n==================== MENU ====================\n");
    printf(" 1. Print the full city graph\n");
    printf(" 2. Run BFS between two cities\n");
    printf(" 3. Run DFS between two cities\n");
    printf(" 4. Run Dijkstra's Algorithm between two cities\n");
    printf(" 5. Run A* Search between two cities\n");
    printf(" 6. Compare Dijkstra vs. A*\n");
    printf(" 7. Compare BFS vs. DFS\n");
    printf(" 0. Exit\n");
    printf("===============================================\n");
    printf("Choice: ");
}

/***********************
          MAIN
************************/

int main(void) {
    if (!loadCitiesFromCSV()) {
        printf("Error: could not find/read canadacities.csv in any known location.\n");
        return 1;
    }

    if (!loadEdgesFromCSV()) {
        printf("Error: could not find/read canada_edges.csv in any known location.\n");
        free(cities);
        return 1;
    }

    char buffer[32];
    int running = 1;

    printf("\nCanadian Route Planner\n");
    printf("Loaded %d cities total (%d with at least one road connection).\n",
           cityCount, cityCount); /* see note in printGraph() re: unconnected cities */

    while (running) {
        printMenu();
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break; /* EOF on stdin */
        }
        killNewline(buffer);
        int choice = atoi(buffer);

        City *start;
        City *goal;

        switch (choice) {
            case 1:
                printGraph();
                break;
            case 2:
                start = promptForCity("Start city: ");
                if (start == NULL) { running = 0; break; }
                goal  = promptForCity("Destination city: ");
                if (goal == NULL) { running = 0; break; }
                runBFS(start, goal);
                break;
            case 3:
                start = promptForCity("Start city: ");
                if (start == NULL) { running = 0; break; }
                goal  = promptForCity("Destination city: ");
                if (goal == NULL) { running = 0; break; }
                runDFS(start, goal);
                break;
            case 4:
                start = promptForCity("Start city: ");
                if (start == NULL) { running = 0; break; }
                goal  = promptForCity("Destination city: ");
                if (goal == NULL) { running = 0; break; }
                printDijkstraResult(start, goal, dijkstra(start, goal));
                break;
            case 5:
                start = promptForCity("Start city: ");
                if (start == NULL) { running = 0; break; }
                goal  = promptForCity("Destination city: ");
                if (goal == NULL) { running = 0; break; }
                printAStarResult(start, goal, aStar(start, goal));
                break;
            case 6:
                start = promptForCity("Start city: ");
                if (start == NULL) { running = 0; break; }
                goal  = promptForCity("Destination city: ");
                if (goal == NULL) { running = 0; break; }
                runComparison(start, goal);
                break;
            case 7:
                start = promptForCity("Start city: ");
                if (start == NULL) { running = 0; break; }
                goal  = promptForCity("Destination city: ");
                if (goal == NULL) { running = 0; break; }
                runBFSvsDFS(start, goal);
                break;
            case 0:
                running = 0;
                break;
            default:
                printf("Invalid choice, try again.\n");
                break;
        }
    }

    freeGraph();
    free(cities);
    cities = NULL;
    printf("Goodbye!\n");
    return 0;
}