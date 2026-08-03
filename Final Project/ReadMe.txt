Canadian Route Planner - Final Project
===========================================
WHAT THIS IS
------------
A graph-based route optimization program in C. Major Canadian cities
are loaded from a CSV file and connected by a small highway network
(also loaded from a CSV file). The program implements and lets you
run/compare four graph algorithms between any two connected cities:

    - BFS               (fewest hops, unweighted)
    - DFS               (a path, not necessarily shortest)
    - Dijkstra's        (shortest real-world distance, binary min-heap)
    - A* Search         (shortest real-world distance, heuristic-guided)

Edge weights are real-world great-circle (Haversine) distances in km,
computed automatically from each city's latitude/longitude.

FILES INCLUDED
--------------
    canada_routes.c              - all source code (single file)
    Resources/canadacities.csv   - city names + coordinates
    Resources/canada_edges.csv   - which cities are road-connected
    README.txt                   - this file


HOW TO COMPILE
---------------
Requires a C11 compiler and a POSIX-ish environment (uses getline()
and strcasecmp(), so Linux or macOS terminal - not required to be
compiled/run in Windows cmd.exe without WSL/MinGW).

From a terminal, in the same folder as canada_routes.c:

    gcc -std=c11 -Wall -Wextra -o canada_routes canada_routes.c -lm

(-lm is required - the program uses <math.h> for the Haversine
distance calculation.)


HOW TO RUN
----------
    ./canada_routes

No command-line arguments are needed - it's a menu-driven program.

USING THE PROGRAM
------------------
On startup you'll see how many cities loaded and how many road
connections were made, then a numbered menu:

    1. Print the full city graph
    2. Run BFS between two cities
    3. Run DFS between two cities
    4. Run Dijkstra's Algorithm between two cities
    5. Run A* Search between two cities
    6. Compare Dijkstra vs. A*
    7. Compare BFS vs. DFS
    0. Exit

Options 2-7 will prompt for a start city and destination city by
name (case-insensitive, e.g. "vancouver" and "Vancouver" both work).

IMPORTANT: canadacities.csv contains many more cities than are
actually road-connected (only the ones in canada_edges.csv have any
roads). Typing a real but unconnected city name is accepted (it's a
recognized city), but every algorithm will report "No path exists"
for it, since it has no edges. For a working demo, use cities from
the road network below - option 1 on the menu will also print this
same list with distances:

    Vancouver, Victoria, Kelowna, Calgary, Edmonton, Saskatoon,
    Regina, Winnipeg, Thunder Bay, Sudbury, Toronto, Ottawa,
    Montreal, Quebec City, Fredericton, Moncton, Halifax

Good pairs to try that show interesting results:
    - Vancouver -> Halifax   (long cross-country route)
    - Kelowna -> Moncton     (BFS and DFS take different-length paths)
    - Any pair, via option 6, to see A* explore fewer cities than
      Dijkstra while agreeing exactly on total distance.

Typing 0 at any time exits the program cleanly and frees all
allocated memory.










