/* pqarray.c */
 
/* Standard Library headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>

/* Constants */
#define INITIAL_CAPACITY 1000
#define MAX_VALUE 1000000000     // max value in randomly generated data
#define MAX_TO_PRINT 100        // cap how many removed numbers we print at once

/* Priority Queue Structure */
typedef struct PriorityQueue {
    int *data;
    size_t size;      /* number of elements currently stored    */
    size_t capacity;  /* size of the backing store (allocation) */
} PriorityQueue;

void init_queue(PriorityQueue *pq) {
    pq->data = (int *)malloc(INITIAL_CAPACITY * sizeof(int));
    if (pq->data == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }
    pq->size = 0;
    pq->capacity = INITIAL_CAPACITY;
}

void free_queue(PriorityQueue *pq) {
    free(pq->data);
    pq->data = NULL;
    pq->size = 0;
    pq->capacity = 0;
}

int compare(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

/* Utility functions */

/* printarray functions print up to MAX_TO_PRINT elements of an array */
void printarray(const int a[], size_t n) {
    size_t numberToPrint = (n < MAX_TO_PRINT) ? n : MAX_TO_PRINT;
    for (size_t i = 0; i < numberToPrint; i++) {
        printf("%d ", a[i]);
    }
    if (n > MAX_TO_PRINT) {
        printf("... (%zu more)", n - MAX_TO_PRINT);
    }
    printf("\n");
}

/* genarray generates an array of 'numberofelements' random integers
   between 0 and MAX_VALUE. Returns a pointer to the array, or NULL on failure.
*/
int *genarray(size_t numberofelements) {
    if (numberofelements == 0) return NULL;

    // overflow check: numberofelements * sizeof(int) must not wrap
    if (numberofelements > SIZE_MAX / sizeof(int)) {
        fprintf(stderr, "Requested %zu elements is too large to allocate.\n", numberofelements);
        return NULL;
    }

    int *result = (int *)malloc(numberofelements * sizeof(int));
    if (result == NULL) {
        fprintf(stderr, "malloc failed for %zu elements.\n", numberofelements);
        return NULL;
    }

    for (size_t i = 0; i < numberofelements; i++) {
        long long int r1 = rand();
        long long int r2 = rand();
        long long int randomValue = (r1 << 14) + (r2 & 0x3fff);
        result[i] = (int)(randomValue % MAX_VALUE);
    }
    return result;
}

/* Priority queue operations */

/* ensure_capacity function ensures that the priority queue has enough capacity
   to hold additional elements. Returns 1 on success, 0 on failure.
*/
int ensure_capacity(PriorityQueue *pq, size_t extra) {
    if (extra == 0) return 1;

    // overflow check: pq->size + extra must not wrap
    if (extra > SIZE_MAX - pq->size) {
        fprintf(stderr, "Requested size overflows addressable range; add rejected.\n");
        return 0;
    }
    size_t required = pq->size + extra;

    if (required <= pq->capacity) return 1; /* already big enough */

    size_t new_capacity = pq->capacity;
    while (new_capacity < required) {
        if (new_capacity > SIZE_MAX / 2) {
            /* doubling would overflow; jump straight to what's needed */
            new_capacity = required;
            break;
        }
        new_capacity *= 2;
    }

    // overflow check: new_capacity * sizeof(int) must not wrap
    if (new_capacity > SIZE_MAX / sizeof(int)) {
        fprintf(stderr, "Requested capacity (%zu elements) is too large to allocate; add rejected.\n", new_capacity);
        return 0;
    }

    int *tmp = (int *)realloc(pq->data, new_capacity * sizeof(int));
    if (tmp == NULL) {
        fprintf(stderr, "Memory allocation failed for %zu elements; queue left unchanged.\n", new_capacity);
        return 0;
    }
    pq->data = tmp;
    pq->capacity = new_capacity;
    return 1;
}

// Insert 'count' already-generated values into the queue and re-sort.
// Returns 1 on success, 0 on failure (queue is left unchanged on failure).
int pq_insert_array(PriorityQueue *pq, const int *values, size_t count) {
    if (count == 0) return 1;
    if (!ensure_capacity(pq, count)) return 0;

    memcpy(pq->data + pq->size, values, count * sizeof(int));
    pq->size += count;
    qsort(pq->data, pq->size, sizeof(int), compare);
    return 1;
}
/* functions to add random numbers and remove/print numbers from the priority queue */
void pq_enqueue_random(PriorityQueue *pq, long long requested) {
    if (requested <= 0) return;
    size_t count = (size_t)requested;

    clock_t startTime = clock();

    int *newvals = genarray(count);
    if (newvals == NULL) {
        fprintf(stderr, "Add of %zu numbers skipped.\n", count);
        return;
    }

    if (!pq_insert_array(pq, newvals, count)) {
        fprintf(stderr, "Add of %zu numbers skipped.\n", count);
        free(newvals);
        return;
    }
    free(newvals);

    clock_t endTime = clock();
    double elapsedTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
    printf("Added %zu numbers. Elapsed time: %f seconds\n", count, elapsedTime);
}
/* pq_dequeue_print removes up to 'requested' numbers from the priority queue,
   prints them, and returns the actual number removed. If requested is larger
   than the current size of the queue, it removes and prints all available numbers.
*/
size_t pq_dequeue_print(PriorityQueue *pq, long long requested) {
    if (requested <= 0 || pq->size == 0) return 0;
    size_t count = (size_t)requested;
    if (count > pq->size) {
        fprintf(stderr, "Only %zu numbers available; removing all of them.\n", pq->size);
        count = pq->size;
    }

    clock_t startTime = clock();

    printarray(pq->data, count);

    size_t remaining = pq->size - count;
    if (remaining > 0) {
        memmove(pq->data, pq->data + count, remaining * sizeof(int));
    }
    pq->size = remaining;

    clock_t endTime = clock();
    double elapsedTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
    printf("Removed %zu numbers. Elapsed time: %f seconds\n", count, elapsedTime);
    return count;
}

/* get_input prompts the user for a number and reads it from stdin.
   It returns 1 if a valid number was read, or 0 if the user entered
   a blank line or EOF. The read number is stored in *value.
*/
int get_input(const char *prompt, long long *value) {
    char buffer[128];

    while (1) {
        printf("%s", prompt);
        fflush(stdout);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            return 0; /* EOF */
        }
        if (buffer[0] == '\n') {
            return 0; /* blank line -> stop */
        }

        /* if the line didn't fit in the buffer, discard the rest of it */
        if (strchr(buffer, '\n') == NULL) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) { /* discard */ }
        }
/* convert the string to a long long int */
        char *endptr;
        errno = 0;
        long long v = strtoll(buffer, &endptr, 10);

        if (endptr == buffer) {
            printf("Please enter a whole number, or press ENTER to stop.\n");
            continue;
        }
        if (errno == ERANGE) {
            printf("That number is out of range. Please enter a smaller number.\n");
            continue;
        }

        *value = v;
        return 1;
    }
}

/* Main loop */
int main() {
    PriorityQueue pq;
    init_queue(&pq);
    srand((unsigned int)time(NULL));

    while (1) {
        long long to_add = 0;
        if (!get_input("How many numbers to add:  ", &to_add)) {
            break;
        }
        pq_enqueue_random(&pq, to_add);

        long long to_remove = 0;
        if (!get_input("How many numbers to remove:  ", &to_remove)) {
            break;
        }
        size_t removed = pq_dequeue_print(&pq, to_remove);

        if (removed > 0 && pq.size == 0) {
            printf("Priority queue is empty. Stopping.\n");
            break;
        }
    }

    free_queue(&pq);
    return 0;
}