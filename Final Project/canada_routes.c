/*
 * canada_routes.c
 *
 * FINAL PROJECT: Finding Shortest Routes Between Major Canadian Cities
 *                Using Graph Algorithms
 *
 * A city network is stored as a graph (adjacency list). Edge weights
 * are real-world great-circle distances (Haversine formula, km).
 *
 * Implements:
 *   - BFS  (unweighted traversal / fewest-hops path)
 *   - DFS  (unweighted traversal / a path, not necessarily shortest)
 *   - Dijkstra's Algorithm (shortest weighted path)
 *   - A* Search            (shortest weighted path, guided by a
 *                            straight-line-distance heuristic)
 *
 * A menu-driven main lets the user run any of the above and compare
 * Dijkstra vs. A* on cities explored / total distance.
 *
 */

#define _POSIX_C_SOURCE 200809L  /* for strcasecmp */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>   /* strcasecmp */
#include <math.h>
#include <float.h>

/***********************
       CONSTANTS
************************/

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_CITIES   30
#define MAX_NAME_LEN 50
#define EARTH_RADIUS 6371.0   /* km */
#define INF          DBL_MAX  /* "infinite" distance, as a double */

/***********************
       DATA STRUCTURES
************************/

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

City cities[MAX_CITIES];
int  cityCount = 0;

/***********************
       CITY CREATION
************************/

void addCity(const char *name, double latitude, double longitude) {
    if (cityCount >= MAX_CITIES) {
        printf("Error: city list is full, cannot add %s\n", name);
        return;
    }
    City *c = &cities[cityCount];
    strncpy(c->name, name, MAX_NAME_LEN - 1);
    c->name[MAX_NAME_LEN - 1] = '\0';
    c->latitude   = latitude;
    c->longitude  = longitude;
    c->connections = NULL;
    c->distance   = INF;
    c->heuristic  = 0.0;
    c->priority   = INF;
    c->visited    = 0;
    c->previous   = NULL;
    cityCount++;
}

City *findCity(const char *name) {
    for (int i = 0; i < cityCount; i++) {
        if (strcasecmp(cities[i].name, name) == 0) {
            return &cities[i];
        }
    }
    return NULL;
}

/* Index of a city pointer within the global cities[] array, used by
 * BFS/DFS to index their own local bookkeeping arrays. */
int cityIndex(City *c) {
    return (int)(c - cities);
}

/***********************
       HAVERSINE FORMULA
************************/

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

/***********************
       GRAPH EDGES
************************/

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

/* Adds a two-way road between two named cities. Distance is computed
 * automatically from their coordinates via Haversine, which also
 * guarantees the A* heuristic (straight-line distance to goal) never
 * overestimates any edge's real cost - i.e. it stays admissible. */
void connectCities(const char *city1, const char *city2) {
    City *a = findCity(city1);
    City *b = findCity(city2);

    if (a == NULL || b == NULL) {
        printf("Warning: could not connect \"%s\" and \"%s\" "
               "(city not found)\n", city1, city2);
        return;
    }

    double distance = haversine(a->latitude, a->longitude,
                                 b->latitude, b->longitude);

    addEdge(a, b, distance);
    addEdge(b, a, distance);
}

/***********************
       CANADIAN DATASET
************************/

void buildCanadianCities() {
    cityCount = 0;

    addCity("Vancouver",    49.2827, -123.1207);
    addCity("Victoria",     48.4284, -123.3656);
    addCity("Kelowna",      49.8880, -119.4960);
    addCity("Calgary",      51.0447, -114.0719);
    addCity("Edmonton",     53.5461, -113.4938);
    addCity("Saskatoon",    52.1332, -106.6700);
    addCity("Regina",       50.4452, -104.6189);
    addCity("Winnipeg",     49.8951,  -97.1384);
    addCity("Thunder Bay",  48.3809,  -89.2477);
    addCity("Sudbury",      46.4917,  -80.9930);
    addCity("Toronto",      43.6532,  -79.3832);
    addCity("Ottawa",       45.4215,  -75.6972);
    addCity("Montreal",     45.5017,  -73.5673);
    addCity("Quebec City",  46.8139,  -71.2080);
    addCity("Fredericton",  45.9636,  -66.6431);
    addCity("Moncton",      46.0878,  -64.7782);
    addCity("Halifax",      44.6488,  -63.5752);
}

/***********************
       BUILD GRAPH
************************/

/* Connections approximate major highway routes. A few redundant links
 * (e.g. a direct Vancouver-Calgary road, and a Toronto/Ottawa/Montreal
 * triangle) are included on purpose: with a plain chain graph, Dijkstra
 * and A* would always explore the exact same cities, which makes the
 * "does A* explore fewer cities?" comparison meaningless. Adding a
 * couple of alternate routes gives A*'s heuristic something to prune. */
void buildGraph() {
    connectCities("Vancouver", "Victoria");
    connectCities("Vancouver", "Kelowna");
    connectCities("Vancouver", "Calgary");     /* direct alternate route */
    connectCities("Kelowna", "Calgary");
    connectCities("Calgary", "Edmonton");
    connectCities("Calgary", "Saskatoon");     /* direct alternate route */
    connectCities("Calgary", "Regina");
    connectCities("Edmonton", "Saskatoon");
    connectCities("Saskatoon", "Regina");
    connectCities("Regina", "Winnipeg");
    connectCities("Winnipeg", "Thunder Bay");
    connectCities("Thunder Bay", "Sudbury");
    connectCities("Sudbury", "Toronto");
    connectCities("Sudbury", "Ottawa");         /* direct alternate route */
    connectCities("Toronto", "Ottawa");
    connectCities("Toronto", "Montreal");       /* direct alternate route */
    connectCities("Ottawa", "Montreal");
    connectCities("Montreal", "Quebec City");
    connectCities("Quebec City", "Fredericton");
    connectCities("Fredericton", "Moncton");
    connectCities("Fredericton", "Halifax");    /* direct alternate route */
    connectCities("Moncton", "Halifax");
}

void freeGraph() {
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

/***********************
       DISPLAY GRAPH
************************/

void printGraph() {
    printf("\nCanadian City Graph\n");
    printf("====================\n\n");

    for (int i = 0; i < cityCount; i++) {
        City *city = &cities[i];
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

/***********************
       SHARED HELPERS
************************/

/* Reset the fields used by Dijkstra/A* before a fresh run. (BFS/DFS
 * keep their own local state and never need this.) */
void resetWeightedState() {
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
       BFS  (unweighted)
************************/

/* Breadth-first search from start to goal. Finds the path with the
 * fewest hops (edges), ignoring distance weights. Fills `path` with
 * the city indices from start to goal (inclusive) and returns the
 * path length, or returns -1 if goal is unreachable. `*citiesExplored`
 * receives the number of cities dequeued during the search. */
int bfs(City *start, City *goal, int path[], int *citiesExplored) {
    int visited[MAX_CITIES]  = {0};
    int previous[MAX_CITIES];
    int queue[MAX_CITIES];
    int front = 0, back = 0;

    for (int i = 0; i < cityCount; i++) previous[i] = -1;

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

    if (!visited[goalIdx]) {
        return -1; /* unreachable */
    }

    /* Walk the previous[] chain backwards, then reverse it into path[] */
    int reversed[MAX_CITIES];
    int len = 0;
    int idx = goalIdx;
    while (idx != -1) {
        reversed[len++] = idx;
        idx = previous[idx];
    }
    for (int i = 0; i < len; i++) {
        path[i] = reversed[len - 1 - i];
    }
    return len;
}

void runBFS(City *start, City *goal) {
    int path[MAX_CITIES];
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
}

/***********************
       DFS  (unweighted)
************************/

/* Depth-first search from start to goal, recursive. Finds *a* path
 * (not guaranteed shortest). Returns 1 if goal is found, 0 otherwise.
 * `path` accumulates the cities visited on the current recursion
 * branch; `*pathLen` tracks how many are currently on it;
 * `*citiesExplored` counts every city dequeued/visited during search. */
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
    int visited[MAX_CITIES] = {0};
    int path[MAX_CITIES];
    int pathLen = 0;
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
}

/***********************
       DIJKSTRA
************************/

City *getClosestUnvisited() {
    City *closest = NULL;
    double smallest = INF;
    for (int i = 0; i < cityCount; i++) {
        if (!cities[i].visited && cities[i].distance < smallest) {
            smallest = cities[i].distance;
            closest  = &cities[i];
        }
    }
    return closest;
}

void relaxEdges(City *current) {
    for (Edge *edge = current->connections; edge != NULL; edge = edge->next) {
        City *neighbor = edge->destination;
        if (neighbor->visited) {
            continue; /* already finalised, nothing to update */
        }
        double newDistance = current->distance + edge->distance;
        if (newDistance < neighbor->distance) {
            neighbor->distance = newDistance;
            neighbor->previous = current;
        }
    }
}

/* Runs Dijkstra from start to goal. Returns the number of cities
 * visited (settled) by the time the goal is reached. The shortest
 * distance ends up in goal->distance, and the route can be printed
 * with printPath(goal). */
int dijkstra(City *start, City *goal) {
    resetWeightedState();
    int citiesExplored = 0;
    start->distance = 0;

    while (1) {
        City *current = getClosestUnvisited();
        if (current == NULL) {
            break; /* nothing left reachable */
        }
        current->visited = 1;
        citiesExplored++;

        if (current == goal) {
            break;
        }
        relaxEdges(current);
    }
    return citiesExplored;
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

/***********************
          A*
************************/

double heuristic(City *current, City *goal) {
    return haversine(current->latitude, current->longitude,
                      goal->latitude, goal->longitude);
}

City *getLowestPriorityCity() {
    City *best = NULL;
    double smallest = INF;
    for (int i = 0; i < cityCount; i++) {
        if (!cities[i].visited && cities[i].priority < smallest) {
            smallest = cities[i].priority;
            best     = &cities[i];
        }
    }
    return best;
}

/* Runs A* from start to goal, guided by the straight-line-distance
 * heuristic to goal. Returns the number of cities visited (settled)
 * by the time the goal is reached. The shortest distance ends up in
 * goal->distance (the real path cost, not the f-score). */
int aStar(City *start, City *goal) {
    resetWeightedState();
    int explored = 0;

    start->distance  = 0;
    start->heuristic = heuristic(start, goal);
    start->priority  = start->distance + start->heuristic;

    while (1) {
        City *current = getLowestPriorityCity();
        if (current == NULL) {
            break;
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
            }
        }
    }
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

/***********************
       COMPARISON
************************/

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

/***********************
       MENU / INPUT HELPERS
************************/

void killNewline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[--len] = '\0';
    }
}

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

void printMenu() {
    printf("\n==================== MENU ====================\n");
    printf(" 1. Print the full city graph\n");
    printf(" 2. Run BFS between two cities\n");
    printf(" 3. Run DFS between two cities\n");
    printf(" 4. Run Dijkstra's Algorithm between two cities\n");
    printf(" 5. Run A* Search between two cities\n");
    printf(" 6. Compare Dijkstra vs. A*\n");
    printf(" 0. Exit\n");
    printf("===============================================\n");
    printf("Choice: ");
}

/***********************
          MAIN
************************/

int main(void) {
    buildCanadianCities();
    buildGraph();

    char buffer[32];
    int running = 1;

    printf("Canadian Route Planner\n");
    printf("Loaded %d cities.\n", cityCount);

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
            case 0:
                running = 0;
                break;
            default:
                printf("Invalid choice, try again.\n");
                break;
        }
    }

    freeGraph();
    printf("Goodbye!\n");
    return 0;
}