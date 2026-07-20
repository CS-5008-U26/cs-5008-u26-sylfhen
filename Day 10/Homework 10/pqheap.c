/* pqheap.c */

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
#define MAX_VALUE 100000000     // max value in randomly generated data
#define MAX_TO_PRINT 100        // cap how many removed numbers we print at once

/* Priority Queue Structure (backed by a min-heap array) */
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

/* Utility functions */

/* printarray prints up to MAX_TO_PRINT elements of an array */
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

/* Heap growth */

/* ensure_capacity makes sure the backing store can hold at least 
   more elements. Returns 1 on success, 0 on failure (overflow or
   allocation failure); on failure the queue is left unchanged.
*/
int ensure_capacity(PriorityQueue *pq, size_t extra) {
    if (extra == 0) return 1;

    if (extra > SIZE_MAX - pq->size) {
        fprintf(stderr, "Requested size overflows addressable range; add rejected.\n");
        return 0;
    }
    size_t required = pq->size + extra;

    if (required <= pq->capacity) return 1; /* already big enough */

    size_t new_capacity = pq->capacity;
    while (new_capacity < required) {
        if (new_capacity > SIZE_MAX / 2) {
            new_capacity = required;
            break;
        }
        new_capacity *= 2;
    }

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

/* Min-heap core */

/* heapify_up restores the heap property by moving the element at index
   up the tree until it is in the correct position. */
void heapify_up(int a[], size_t index) {
    while (index > 0) {
        size_t parent = (index - 1) / 2;
        if (a[index] < a[parent]) {
            int tmp = a[index];
            a[index] = a[parent];
            a[parent] = tmp;
            index = parent;
        } else {
            break;
        }
    }
}

/* heapify_down assumes the heap is valid everywhere except possibly at 'here' */
void heapify_down(int a[], size_t n, size_t index) {
    while (1) {
        size_t left = 2 * index + 1;
        size_t right = 2 * index + 2;
        size_t smallest = index;

        if (left < n && a[left] < a[smallest]) {
            smallest = left;
        }
        if (right < n && a[right] < a[smallest]) {
            smallest = right;
        }
        if (smallest == index) {
            break;
        }
        int tmp = a[index];
        a[index] = a[smallest];
        a[smallest] = tmp;
        index = smallest;
    }
}

/* insertValue function add newValue to the heap and restore the heap property */
int insert_value(PriorityQueue *pq, int newValue) {
    if (!ensure_capacity(pq, 1)) return 0;
    pq->data[pq->size] = newValue;
    pq->size += 1;
    heapify_up(pq->data, pq->size - 1);
    return 1;
}

/* extract_min function remove and return the smallest value from the heap */
int extract_min(PriorityQueue *pq) {
    int minValue = pq->data[0];
    size_t last = pq->size - 1;
    pq->data[0] = pq->data[last];
    pq->size = last;
    if (pq->size > 0) {
        heapify_down(pq->data, pq->size, 0);
    }
    return minValue;
}

/* Enqueue / dequeue batches */

/* pq_enqueue_random generates 'requested' random numbers and inserts them
   into the heap one at a time (heapify_up per insertion), per the
   assignment's description of enqueue. */
void pq_enqueue_random(PriorityQueue *pq, long long requested) {
    if (requested <= 0) return;
    size_t count = (size_t)requested;

    clock_t startTime = clock();

    int *newvals = genarray(count);
    if (newvals == NULL) {
        fprintf(stderr, "Add of %zu numbers skipped.\n", count);
        return;
    }

    /* Reserve space for the whole batch up front so we're not
       reallocating on every single insert -- the heapify_up calls
       themselves still happen one at a time, per the spec. */
    if (!ensure_capacity(pq, count)) {
        fprintf(stderr, "Add of %zu numbers skipped.\n", count);
        free(newvals);
        return;
    }

    for (size_t i = 0; i < count; i++) {
        pq->data[pq->size] = newvals[i];
        pq->size += 1;
        heapify_up(pq->data, pq->size - 1);
    }
    free(newvals);

    clock_t endTime = clock();
    double elapsedTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
    printf("Added %zu numbers. Elapsed time: %f seconds\n", count, elapsedTime);
}

/* pq_dequeue_print removes up to 'requested' numbers from the heap one
   extract-min at a time (which comes out in ascending order for free),
   prints them (capped at MAX_TO_PRINT, matching printarray's style), and
   returns the number actually removed. */
size_t pq_dequeue_print(PriorityQueue *pq, long long requested) {
    if (requested <= 0 || pq->size == 0) return 0;
    size_t count = (size_t)requested;
    if (count > pq->size) {
        fprintf(stderr, "Only %zu numbers available; removing all of them.\n", pq->size);
        count = pq->size;
    }

    clock_t startTime = clock();

    size_t printed = 0;
    for (size_t i = 0; i < count; i++) {
        int value = extract_min(pq);
        if (printed < MAX_TO_PRINT) {
            printf("%d ", value);
            printed++;
        }
    }
    if (count > MAX_TO_PRINT) {
        printf("... (%zu more)", count - MAX_TO_PRINT);
    }
    printf("\n");

    clock_t endTime = clock();
    double elapsedTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
    printf("Removed %zu numbers. Elapsed time: %f seconds\n", count, elapsedTime);
    return count;
}

/* Input handling */

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