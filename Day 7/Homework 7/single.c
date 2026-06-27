/*Linked List Implementation*/

/*Standard Library Includes*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/*
 * Struct to store city record fields.
 * name, state_id, and population.
 * nameASCII is kept because the CSV column order requires us to
 * parse past it to reach state_id.
 */
typedef struct city {
    char *name;
    char *nameASCII;
    char *stateID;
    long  population;
} city;

/*
 * Strip trailing newline (and carriage return) from str.
 * We check for \r after \n to handle both Unix (\n) and
 * Windows (\r\n) line endings, preventing field corruption.
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
 * The CSV standard wraps fields containing commas in quotes;
 * we strip them so callers receive clean values.
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
 * field (e.g. the zip-codes column) are not treated as separators.
 * Returns a pointer to the character just after the separator,
 * or to the end of the string if no separator was found.
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
            inQuotes = !inQuotes;       /* Toggle quote state. */
        } else if (*cursor == separator && !inQuotes) {
            break;                      /* Unquoted separator ends the field. */
        }
        *write++ = *cursor++;
    }
    *write = '\0';
    stripEnclosingQuotes(out);

    /* Advance past the separator so the next call starts at the next field. */
    if (*cursor == separator)
        return cursor + 1;

    return cursor;
}

/*
 * Allocate a heap copy of src.
 * Centralising this prevents repeated malloc+strcpy patterns
 * and makes NULL-checks easy to add in one place.
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
 * Parse a CSV line into a heap allocated city struct.
 * Column order in uscities.csv:
 *   0: city   1: city_ascii   2: state_id   3: state_name
 *   4: county_fips   5: county_name   6: lat   7: lng
 *   8: population   and other columns we ignore.
 * We skip columns we do not need by calling getNextField and
 * discarding the result into the scratch buffer.
 */
city *stringToCity(char *line) {
    if (line == NULL)
        return NULL;

    city *c = malloc(sizeof(city));
    if (c == NULL)
        return NULL;

    char  field[1000];
    char *cursor = line;

    /* Column 0: city name */
    cursor    = getNextField(cursor, ',', field);
    c->name   = copyString(field);

    /* Column 1: ASCII city name */
    cursor       = getNextField(cursor, ',', field);
    c->nameASCII = copyString(field);

    /* Column 2: state abbreviation (e.g. "NY") */
    cursor     = getNextField(cursor, ',', field);
    c->stateID = copyString(field);

    /* Columns 3 to 7: skip state_name, county_fips, county_name, lat, lng */
    cursor = getNextField(cursor, ',', field); /* state_name  */
    cursor = getNextField(cursor, ',', field); /* county_fips */
    cursor = getNextField(cursor, ',', field); /* county_name */
    cursor = getNextField(cursor, ',', field); /* lat         */
    cursor = getNextField(cursor, ',', field); /* lng         */

    /* Column 8: population parse as long integer */
    cursor       = getNextField(cursor, ',', field);
    c->population = atol(field);

    /* If any heap allocation failed, free everything and return NULL. */
    if (c->name == NULL || c->nameASCII == NULL || c->stateID == NULL) {
        free(c->name);
        free(c->nameASCII);
        free(c->stateID);
        free(c);
        return NULL;
    }

    return c;
}

/*
 * Print a single city in the appropriate format required:
 *   "Chicago IL, population 8489066"
 */
void printCity(city *c) {
    if (c == NULL)
        return;
    printf("%s %s, population %ld\n", c->name, c->stateID, c->population);
}

/*
 * Singly-linked list node.
 * Using void* for data makes the node type-agnostic, so the same
 * struct and functions can hold any heap allocated data (city*,
 * int*, etc.) without rewriting list logic for each type.
 */
typedef struct singleNode {
    void *data;
    struct singleNode *next;
} sNode;

/*
 * Allocate and initialise a new list node wrapping the given data pointer.
 * Returning NULL on failure detects the error cleanly
 * rather than dereferencing a bad pointer.
 */
sNode *createNode(void *data) {
    if (data == NULL)
        return NULL;

    sNode *node = malloc(sizeof(sNode));

    /* malloc can fail on OOM; propagate the failure rather than segfault. */
    if (node == NULL)
        return NULL;

    node->data = data;
    node->next = NULL;  /* NULL terminate so list traversal is safe. */

    return node;
}

/*
 * Insert node at the head of the list.
 * O(1) operation — only a fixed number of pointer updates regardless of list length.
 * We accept a double pointer so the caller's head variable is updated in place.
 */
void addFront(sNode **list, sNode *node) {
    if (list == NULL || node == NULL)
        return;

    node->next = *list;  /* Chain new node onto the existing list. */
    *list = node;        /* Advance the caller's head to the new node. */
}

/*
 * Append node at the tail of the list.
 * O(n) operation — we must traverse to the end.
 * Used during file loading so cities appear in the same order as in the CSV.
 */
void addEnd(sNode **list, sNode *node) {
    if (list == NULL || node == NULL)
        return;

    /* Empty list: the new node is both head and tail. */
    if (*list == NULL) {
        *list = node;
        return;
    }

    /* Traverse to the last node, then attach. */
    sNode *current = *list;
    while (current->next != NULL)
        current = current->next;

    current->next = node;
}

/*
 * Return a pointer to the nth node.
 * Returning NULL for out-of-range n validity check
 * without needing to know the list length in advance.
 */
sNode *getNode(sNode *list, int n) {
    if (n <= 0)
        return NULL;

    int count = 1;
    while (list != NULL && count < n) {
        list = list->next;
        count++;
    }

    /* Returns NULL naturally if the list is shorter than n. */
    return list;
}

/*
 * Unlink and free the given node and its city payload.
 * Centralising deallocation here (rather than in callers) ensures
 * every code path that removes a node also frees its memory,
 * preventing leaks and double frees.
 */
void deleteNode(sNode **list, sNode *node) {
    if (list == NULL || *list == NULL || node == NULL)
        return;

    if (*list == node) {
        /* Removing the head: advance the head pointer past it. */
        *list = node->next;
    } else {
        /* Traverse until we find the node just before the target. */
        sNode *prev = *list;
        while (prev->next != NULL && prev->next != node)
            prev = prev->next;

        /* Node was not found in this list; nothing to do. */
        if (prev->next == NULL)
            return;

        prev->next = node->next;  /* Bypass the target node. */
    }

    /* Free the city payload before the node itself. */
    city *c = (city *)node->data;
    free(c->name);
    free(c->nameASCII);
    free(c->stateID);
    free(c);
    free(node);
}

/*
 * Return the number of nodes in the list.
 * O(n) Traversal acceptable because "size" is a user command,
 * not an inner-loop operation.
 */
int listLength(sNode *list) {
    int count = 0;
    while (list != NULL) {
        count++;
        list = list->next;
    }
    return count;
}

/*
 * Reverse the list in place using three-pointer iteration, O(n).
 * We re-wire next pointers rather than copying data so this works
 * for any payload type without any extra allocation.
 */
void reverseList(sNode **list) {
    if (list == NULL)
        return;

    sNode *prev    = NULL;
    sNode *current = *list;
    sNode *next    = NULL;

    while (current != NULL) {
        next          = current->next;  /* Save forward link before overwriting. */
        current->next = prev;           /* Reverse the pointer direction. */
        prev          = current;        /* Advance prev to the node just processed. */
        current       = next;           /* Move forward in the original order. */
    }

    *list = prev;  /* prev is now the new head (was the old tail). */
}


/* File I/O Implementations */

/*
 * Read the first 20 cities from filename into a linked list and return it.
 * We cap at 20 to match the requirements.
 * The first CSV line is a header row, so we consume it before the data loop.
 */
sNode *readCityList(char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL)
        return NULL;

    char  *buffer   = NULL;
    size_t capacity = 0;

    /* Discard the header line; it is metadata, not city data. */
    if (getline(&buffer, &capacity, file) == -1) {
        fclose(file);
        free(buffer);
        return NULL;
    }

    sNode *list = NULL;
    int    count = 0;

    while (count < 20 && getline(&buffer, &capacity, file) != -1) {
        killNewline(buffer);  /* Strip trailing newline before parsing. */

        city  *c    = stringToCity(buffer);
        if (c == NULL)
            continue;         /* Skip malformed lines rather than inserting NULL. */

        sNode *node = createNode(c);
        addEnd(&list, node);  /* Append to preserve file order. */
        count++;
    }

    fclose(file);
    free(buffer);
    return list;
}

/*
 * Print city data for the first n nodes.
 * Stops at n so the user can preview a subset without
 * modifying the list structure.
 */
void printFirstN(sNode *list, int n) {
    int count = 0;
    while (list != NULL && count < n) {
        printCity((city *)list->data);
        list = list->next;
        count++;
    }
}

/*
 * Move the nth node to the front of the list.
 * Short circuits for n <= 1 because node 1 is already the head
 * moving it would be a no-op and the loop below requires a valid prev.
 */
void moveToFront(sNode **list, int n) {
    if (list == NULL || *list == NULL || n <= 1)
        return;

    sNode *prev    = NULL;
    sNode *current = *list;
    int    count   = 1;

    /* Traverse to the nth node, keeping track of its predecessor. */
    while (current != NULL && count < n) {
        prev    = current;
        current = current->next;
        count++;
    }

    /* n was larger than the list length; nothing to promote. */
    if (current == NULL)
        return;

    /* Unlink current from its current position... */
    prev->next = current->next;

    /* then splice it in as the new head. */
    current->next = *list;
    *list = current;
}

/*
 * Delete the nth node from the list.
 * Thin wrapper: resolve n to a pointer first, then delegate to
 * deleteNode which owns all deallocation logic.
 */
void deleteNth(sNode **list, int n) {
    sNode *node = getNode(*list, n);
    deleteNode(list, node);
}


/* Main method */

int main(void) {
    sNode *list = readCityList("../../Resources/uscities.csv");

    /*
     * numBuf holds the raw text from fgets when reading an integer.
     * Using fgets + atoi keeps stdin consistent:
     * fgets always consumes the newline, so there is no residual '\n'
     * left to corrupt the next command read.
     */
    char command[100];
    char numBuf[32];

    while (1) {
        printf("\nsize, delete, reverse, get, or print: ");

        /* fgets returns NULL on EOF (e.g. Ctrl-D); treat as exit signal. */
        if (fgets(command, sizeof(command), stdin) == NULL)
            break;
        killNewline(command);

        if (strcmp(command, "size") == 0) {
            printf("Size is %d\n", listLength(list));
        }

        else if (strcmp(command, "delete") == 0) {
            printf("Enter a number: ");
            if (fgets(numBuf, sizeof(numBuf), stdin) == NULL)
                break;
            deleteNth(&list, atoi(numBuf));
        }

        else if (strcmp(command, "reverse") == 0) {
            reverseList(&list);
        }

        else if (strcmp(command, "get") == 0) {
            printf("Enter a number: ");
            if (fgets(numBuf, sizeof(numBuf), stdin) == NULL)
                break;
            moveToFront(&list, atoi(numBuf));
        }

        else if (strcmp(command, "print") == 0) {
            printf("Enter a number: ");
            if (fgets(numBuf, sizeof(numBuf), stdin) == NULL)
                break;
            printFirstN(list, atoi(numBuf));
        }

        else {
            /* Any unrecognised command exits the loop gracefully. */
            break;
        }
    }

    /*
     * Traverse the list and free every node's payload before freeing the node.
     * We save list->next before freeing because reading freed memory
     * is undefined behaviour.
     */
    while (list != NULL) {
        sNode *temp = list;
        list = list->next;

        city *c = (city *)temp->data;
        free(c->name);
        free(c->nameASCII);
        free(c->stateID);
        free(c);
        free(temp);
    }

    return 0;
}