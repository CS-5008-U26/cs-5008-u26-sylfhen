/*Linked List Implementation*/

/*Standard Library Includes*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Struct to store city record fields */
typedef struct city {
    char *name;
    char *nameASCII;
} city;

void killNewline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n')
        str[len - 1] = '\0';
}

/* Extract the next comma-separated field from 'start' into 'out'.
 * Returns a pointer to the character just after the separator (or end of string).
 */
static char *getNextField(char *start, char separator, char *out) {
    if (*start == '\0') {
        out[0] = '\0';
        return start;
    }

    char *sepPos = strchr(start, separator);
    if (sepPos == NULL) {
        strcpy(out, start);
        return start + strlen(start);
    }

    size_t fieldLength = (size_t)(sepPos - start);
    strncpy(out, start, fieldLength);
    out[fieldLength] = '\0';
    return sepPos + 1;
}

static char *copyString(const char *src) {
    size_t len = strlen(src);
    char *copy = malloc(len + 1);
    if (copy == NULL)
        return NULL;

    strcpy(copy, src);
    return copy;
}

city *stringToCity(char *line) {
    if (line == NULL)
        return NULL;

    city *c = malloc(sizeof(city));
    if (c == NULL)
        return NULL;

    char field[1000];
    char *cursor = line;

    cursor = getNextField(cursor, ',', field);
    c->name = copyString(field);

    cursor = getNextField(cursor, ',', field);
    c->nameASCII = copyString(field);

    if (c->name == NULL || c->nameASCII == NULL) {
        free(c->name);
        free(c->nameASCII);
        free(c);
        return NULL;
    }

    return c;
}

void printCity(city *c) {
    if (c == NULL)
        return;

    printf("%s (%s)\n", c->name, c->nameASCII);
}

/*
 * singly-linked list node.
 *
 * Using void* for data is ideal because it makes the node type-agnostic, so the same
 * struct and functions can hold any heap-allocated data (city*,
 * int*, etc.) without rewriting list logic for each type.
 */
typedef struct singleNode {
    void *data;
    struct singleNode *next;
} sNode;

/*
 * Allocating and initialising a new list node that wraps the given data pointer.
 * returning NULL is a check to detect the error cleanly.
 */
sNode *createNode(void *data) {
    if (data == NULL)
        return NULL;

    sNode *node = malloc(sizeof(sNode));

    /* malloc can fail on OOM; propagate the failure rather than segfault. */
    if (node == NULL)
        return NULL;

    node->data = data;
    node->next = NULL;   /* Always NULL-terminate so list traversal is safe. */

    return node;
}

/*
 * Inserting node at the head of the list.
 * Using a double pointer so that the head variable is updated in place.
 * O(1) time complexity because we only change a few pointers, regardless of list length.
 */
void addFront(sNode **list, sNode *node) {
    if (list == NULL || node == NULL)
        return;

    node->next = *list;   /* Chain new node onto the existing list. */
    *list = node;         /* Advance the caller's head to the new node. */
}

/*
 * Append node at the tail of the list.
 * O(n) time complexity because we traverse to the end.
 * Ensures that cities appear in the same order as in the CSV file.
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
 * Returning NULL for out of range validity check.
 * Especially without knowing the list length up front.
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
 * Unlink and free the given node from the list.
 */
void deleteNode(sNode **list, sNode *node) {
    if (list == NULL || *list == NULL || node == NULL)
        return;

    if (*list == node) {
        /* Removing the head: just advance the head pointer. */
        *list = node->next;
    } else {
        /* Traverse until we find the node just before the target. */
        sNode *prev = *list;
        while (prev->next != NULL && prev->next != node)
            prev = prev->next;

        /* Node was not found in this list; nothing to do. */
        if (prev->next == NULL)
            return;

        prev->next = node->next;   /* Bypass the target node. */
    }

    /* Free the city node. */
    city *c = (city *)node->data;
    free(c->name);
    free(c->nameASCII);
    free(c);
    free(node);
}

/*
 * Return the number of nodes in the list.
 */
int listLength(sNode *list) {
    int count = 0;
    while (list != NULL) {
        count++;
        list = list->next;
    }
    return count;
}

/* Reverse the list in place, O(n) operations. */
void reverseList(sNode **list) {
    if (list == NULL)
        return;

    sNode *prev    = NULL;
    sNode *current = *list;
    sNode *next    = NULL;

    while (current != NULL) {
        next           = current->next;  /* Save the forward link before overwriting it. */
        current->next  = prev;           /* Reverse the pointer. */
        prev           = current;        /* Advance prev to the node we just processed. */
        current        = next;           /* Move forward in the original order. */
    }

    *list = prev;   /* prev is now the new head (was the old tail). */
}


/* File I/O */

/*
 * Read the first 20 cities from filename into a linked list and return it.
 * the CSV is a header row, so consuming it with fgets before the loop.
 */
sNode *readCityList(char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL)
        return NULL;

    char buffer[1000];

    /* Discarding the header line. */
    fgets(buffer, sizeof(buffer), file);

    sNode *list = NULL;
    int count   = 0;

    while (count < 20 && fgets(buffer, sizeof(buffer), file) != NULL) {
        killNewline(buffer);        /* Strip trailing '\n' before parsing. */

        city  *c    = stringToCity(buffer);
        sNode *node = createNode(c);
        addEnd(&list, node);        /* Preserve file order by always appending. */
        count++;
    }

    fclose(file);
    return list;
}

/*
 * Print the city data for the first n nodes in the list.
 * Stopping at n (rather than printing the whole list) lets the user
 * preview a subset without modifying the list structure.
 */
void printFirstN(sNode *list, int n) {
    int count = 0;
    while (list != NULL && count < n) {
        printCity((city *)list->data);
        list = list->next;
        count++;
    }
}

 /* Move the n-th node to the front of the list.*/
 /* Short-circuiting for n <= 1 because the first node is already at the front. */
void moveToFront(sNode **list, int n) {
    if (list == NULL || *list == NULL || n <= 1)
        return;

    sNode *prev    = NULL;
    sNode *current = *list;
    int    count   = 1;

    /* Walk to the n-th node, keeping track of its predecessor. */
    while (current != NULL && count < n) {
        prev = current;
        current = current->next;
        count++;
    }

    /* Checking if n was larger than the list length; nothing to promote. */
    if (current == NULL)
        return;

    /* Unlink current from its position. */
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

    /* Command loop: prompt for a command, then execute it. */
    char command[100];
    char numBuf[32];

    while (1) {
        printf("\nsize, delete, reverse, get, or print: ");

        /* fgets includes the newline in the buffer; killNewline strips it. */
        fgets(command, sizeof(command), stdin);
        killNewline(command);

        if (strcmp(command, "size") == 0) {
            printf("Size is %d\n", listLength(list));
        }

        else if (strcmp(command, "delete") == 0) {
            printf("Enter a number: ");
            fgets(numBuf, sizeof(numBuf), stdin);
            int n = atoi(numBuf);   
            deleteNth(&list, n);
        }

        else if (strcmp(command, "reverse") == 0) {
            reverseList(&list);
        }

        else if (strcmp(command, "get") == 0) {
            printf("Enter a number: ");
            fgets(numBuf, sizeof(numBuf), stdin);
            int n = atoi(numBuf);
            moveToFront(&list, n);
        }

        else if (strcmp(command, "print") == 0) {
            printf("Enter a number: ");
            fgets(numBuf, sizeof(numBuf), stdin);
            int n = atoi(numBuf);
            printFirstN(list, n);
        }

        else {
            /* Any unrecognised command exits the loop gracefully. */
            break;
        }
    }

    /* Traverse the list and free each node's payload before freeing the node itself. */
    while (list != NULL) {
        sNode *temp = list;
        list = list->next;

        city *c = (city *)temp->data;
        free(c->name);
        free(c->nameASCII);
        free(c);
        free(temp);
    }

    return 0;
}