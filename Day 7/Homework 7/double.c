/*Doubly-Linked List Implementation*/

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
 *   0: city   1: city_ascii   2: state_id   3: state_name
 *   4: county_fips   5: county_name   6: lat   7: lng
 *   8: population   and other columns we ignore.
 */
city *stringToCity(char *line) {
    if (line == NULL)
        return NULL;

    city *c = malloc(sizeof(city));
    if (c == NULL)
        return NULL;

    char  field[1000];
    char *cursor = line;

    cursor    = getNextField(cursor, ',', field);
    c->name   = copyString(field);

    cursor       = getNextField(cursor, ',', field);
    c->nameASCII = copyString(field);

    cursor     = getNextField(cursor, ',', field);
    c->stateID = copyString(field);

    cursor = getNextField(cursor, ',', field); /* state_name  */
    cursor = getNextField(cursor, ',', field); /* county_fips */
    cursor = getNextField(cursor, ',', field); /* county_name */
    cursor = getNextField(cursor, ',', field); /* lat         */
    cursor = getNextField(cursor, ',', field); /* lng         */

    cursor        = getNextField(cursor, ',', field);
    c->population = atol(field);

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
 * Print a single city: "Chicago IL, population 8489066"
 */
void printCity(city *c) {
    if (c == NULL)
        return;
    printf("%s %s, population %ld\n", c->name, c->stateID, c->population);
}

/*
 * Doubly-linked list node.
 * Each node carries both a forward (next) and backward (prev) pointer,
 * allowing O(1) removal without needing to traverse to find the predecessor.
 */
typedef struct doubleNode {
    void *data;
    struct doubleNode *next;
    struct doubleNode *prev;
} dNode;

/*
 * Allocate and initialise a new doubly-linked list node.
 * Both next and prev are set to NULL — the caller links the node in.
 */
dNode *createNode(void *data) {
    if (data == NULL)
        return NULL;

    dNode *node = malloc(sizeof(dNode));
    if (node == NULL)
        return NULL;

    node->data = data;
    node->next = NULL;
    node->prev = NULL;

    return node;
}

/*
 * Insert node at the head of the list.
 * O(1): updates the old head's prev pointer and the caller's head variable.
 */
void addFront(dNode **list, dNode *node) {
    if (list == NULL || node == NULL)
        return;

    node->next = *list;
    node->prev = NULL;

    if (*list != NULL)
        (*list)->prev = node;   /* Back-link the old head to the new node. */

    *list = node;
}

/*
 * Append node at the tail of the list.
 * O(n): traverses to the end, then links in both directions.
 */
void addEnd(dNode **list, dNode *node) {
    if (list == NULL || node == NULL)
        return;

    if (*list == NULL) {
        *list = node;
        return;
    }

    dNode *current = *list;
    while (current->next != NULL)
        current = current->next;

    current->next = node;
    node->prev    = current;  /* Back-link to the previous tail. */
    node->next    = NULL;
}

/*
 * Return a pointer to the nth node (1-based).
 * Returns NULL when n is out of range.
 */
dNode *getNode(dNode *list, int n) {
    if (n <= 0)
        return NULL;

    int count = 1;
    while (list != NULL && count < n) {
        list = list->next;
        count++;
    }

    return list;
}

/*
 * Unlink and free the given node and its city payload.
 * Because each node knows its prev, we do not need to search for the
 * predecessor — removal is O(1) once the node pointer is in hand.
 */
void deleteNode(dNode **list, dNode *node) {
    if (list == NULL || *list == NULL || node == NULL)
        return;

    /* Re-wire the predecessor's next pointer. */
    if (node->prev != NULL)
        node->prev->next = node->next;
    else
        *list = node->next;     /* node was the head; advance the head. */

    /* Re-wire the successor's prev pointer. */
    if (node->next != NULL)
        node->next->prev = node->prev;

    city *c = (city *)node->data;
    free(c->name);
    free(c->nameASCII);
    free(c->stateID);
    free(c);
    free(node);
}

/*
 * Return the number of nodes in the list. O(n).
 */
int listLength(dNode *list) {
    int count = 0;
    while (list != NULL) {
        count++;
        list = list->next;
    }
    return count;
}

/*
 * Reverse the list in place by swapping each node's next and prev pointers.
 * After the loop, the old tail (prev) becomes the new head.
 */
void reverseList(dNode **list) {
    if (list == NULL)
        return;

    dNode *current = *list;
    dNode *temp    = NULL;

    while (current != NULL) {
        /* Swap next and prev for the current node. */
        temp          = current->prev;
        current->prev = current->next;
        current->next = temp;

        /* Move to the next node in the original order (now stored in prev). */
        current = current->prev;
    }

    /* After the loop, temp points to the node whose original next was NULL,
       i.e. the old tail, which is now the new head. */
    if (temp != NULL)
        *list = temp->next;     /* temp->next is the old tail after the swap. */
}

/* File I/O Implementations */

/*
 * Read the first 20 cities from filename into a doubly-linked list.
 */
dNode *readCityList(char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL)
        return NULL;

    char  *buffer   = NULL;
    size_t capacity = 0;

    if (getline(&buffer, &capacity, file) == -1) {
        fclose(file);
        free(buffer);
        return NULL;
    }

    dNode *list  = NULL;
    int    count = 0;

    while (count < 20 && getline(&buffer, &capacity, file) != -1) {
        killNewline(buffer);

        city  *c    = stringToCity(buffer);
        if (c == NULL)
            continue;

        dNode *node = createNode(c);
        addEnd(&list, node);
        count++;
    }

    fclose(file);
    free(buffer);
    return list;
}

/*
 * Print city data for the first n nodes.
 */
void printFirstN(dNode *list, int n) {
    int count = 0;
    while (list != NULL && count < n) {
        printCity((city *)list->data);
        list = list->next;
        count++;
    }
}

/*
 * Move the nth node to the front of the list.
 * With a doubly-linked list the predecessor is found via node->prev,
 * so we do not need a separate prev-tracking pointer during traversal.
 */
void moveToFront(dNode **list, int n) {
    if (list == NULL || *list == NULL || n <= 1)
        return;

    dNode *node = getNode(*list, n);
    if (node == NULL)
        return;

    /* Unlink node from its current position. */
    if (node->prev != NULL)
        node->prev->next = node->next;
    if (node->next != NULL)
        node->next->prev = node->prev;

    /* Splice node in as the new head. */
    node->prev    = NULL;
    node->next    = *list;
    (*list)->prev = node;
    *list         = node;
}

/*
 * Delete the nth node from the list.
 */
void deleteNth(dNode **list, int n) {
    dNode *node = getNode(*list, n);
    deleteNode(list, node);
}


/* Main method */

int main(void) {
    dNode *list = readCityList("../../Resources/uscities.csv");

    char command[100];
    char numBuf[32];

    while (1) {
        printf("\nsize, delete, reverse, get, or print: ");

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
            break;
        }
    }

    /* Free all nodes and their city payloads. */
    while (list != NULL) {
        dNode *temp = list;
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
