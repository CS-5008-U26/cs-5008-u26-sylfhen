/* bstsort.c */

/* Standard Library includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_TO_PRINT 100        // max number of elements to print
#define MAX_VALUE 100000000     // max value in randomly generated data

/* T E S T   C A S E   D A T A */

int test1[] =       { 3 };
int expected1[] =   { 3 };
int n1 =            1;
int test2[] =       { 5, 3 };
int expected2[] =   { 3, 5 };
int n2 =            2;
int test3[] =       { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
int expected3[] =   { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
int n3 =            10;
int test4[] =       { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };
int expected4[] =   { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
int n4 =            10;
int test5[] =       { 2, 4, 6, 8, 10, 1, 3, 5, 7, 9 };
int expected5[] =   { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
int n5 =            10;
int test6[] =       { 1, 2, 3, 2, 2 };
int expected6[] =   { 1, 2, 2, 2, 3 };
int n6 =            5;

/* Y O U R   S O R T   F U N C T I O N */

typedef struct bstNode_struct {
    int value;
    struct bstNode_struct *left;
    struct bstNode_struct *right;
} BSTNode;

static BSTNode *createBSTNode(int value) {
    BSTNode *newNode = malloc(sizeof(BSTNode));
    if (newNode == NULL) {
        printf("malloc failed");
        exit(1);
    }
    newNode->value = value;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

/* inserts a value into the binary search tree rooted at root.
   Returns the (possibly new) root of the tree. */
static BSTNode *bstInsert(BSTNode *root, int value) {
    BSTNode *newNode = createBSTNode(value);

    if (root == NULL) {
        return newNode;
    }

    BSTNode *current = root;
    while (1) {
        if (value < current->value) {
            if (current->left == NULL) {
                current->left = newNode;
                break;
            }
            current = current->left;
        } else {
            if (current->right == NULL) {
                current->right = newNode;
                break;
            }
            current = current->right;
        }
    }

    return root;
}

/* inorderTraversal - iterative in-order traversal using an explicit,
   heap-allocated stack instead of the call stack, to avoid stack
   overflow on a completely unbalanced tree (from sorted or
   reverse-sorted input). */
static void inorderTraversal(BSTNode *root, int a[], int *index, int n) {
    if (n <= 0) {
        return;
    }

    BSTNode **stack = malloc(n * sizeof(BSTNode *));
    if (stack == NULL) {
        printf("malloc failed");
        exit(1);
    }

    int top = -1;
    BSTNode *current = root;

    while (current != NULL || top >= 0) {
        while (current != NULL) {
            stack[++top] = current;
            current = current->left;
        }
        current = stack[top--];
        a[*index] = current->value;
        (*index)++;
        current = current->right;
    }

    free(stack);
}

/* Frees the memory used by the binary search tree rooted at root.*/
static void freeBST(BSTNode *root, int n) {
    if (root == NULL || n <= 0) {
        return;
    }

    BSTNode **stack = malloc(n * sizeof(BSTNode *));
    if (stack == NULL) {
        printf("malloc failed");
        exit(1);
    }

    int top = -1;
    stack[++top] = root;

    while (top >= 0) {
        BSTNode *node = stack[top--];
        if (node->left != NULL) {
            stack[++top] = node->left;
        }
        if (node->right != NULL) {
            stack[++top] = node->right;
        }
        free(node);
    }

    free(stack);
}

void sortarray(int a[], int n) {
    BSTNode *root = NULL;
    for (int i = 0; i < n; i++) {
        root = bstInsert(root, a[i]);
    }
    int index = 0;
    inorderTraversal(root, a, &index, n);
    //freeBST(root, n);
}

/* U T I L I T Y   F U N C T I O N S */

int *genarray(int numberofelements) {
    int *result = malloc(numberofelements * sizeof(int));
    if (result == NULL) {
        printf("malloc failed");
    } else {
        // fill the result array with random numbers between 0 and MAX_VALUE
        for (int i = 0; i < numberofelements; i++) {
            // we have a problem
            // On Windows the rand() function only gives a 15-bit random number
            // This will be between 0 and 32767
            // So we will fix this with a crude trick
            // We will generate two random numbers, one for the high-order bits and the other
            // for the low-order 14 bits
            long long int r1 = rand();
            long long int r2 = rand();
            long long int randomValue = (r1 << 14) + (r2 & 0x3fff);
            result[i] = (int)(randomValue % MAX_VALUE);
        }
    }
    return (result);
}

/* Generates a sorted array */
int *gensortedarray(int numberofelements) {
    int *result = malloc(numberofelements * sizeof(int));
    if (result == NULL) {
        printf("malloc failed");
    } else {
        for (int i = 0; i < numberofelements; i++) {
            result[i] = i;
        }
    }
    return (result);
}

/* Generates a reverse-sorted array */
int *genreversearray(int numberofelements) {
    int *result = malloc(numberofelements * sizeof(int));
    if (result == NULL) {
        printf("malloc failed");
    } else {
        for (int i = 0; i < numberofelements; i++) {
            result[i] = numberofelements - 1 - i;
        }
    }
    return (result);
}

/* Prints an array of integers */
void printarray(int a[], int n) {
    int numberToPrint = ((n < MAX_TO_PRINT) ? n : MAX_TO_PRINT);
    for (int i = 0; i < numberToPrint; i++) {
        printf("%d ", a[i]);
    }
    printf("\n");
}
/* Runs a sort and records the elapsed time */
void timedsort(int a[], int n) {
    clock_t startTime = clock();        // get the start time
    sortarray(a, n);
    clock_t endTime = clock();          // get the end time
    double elapsedTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
    printf("Result: ");                 // print the sorted data
    printarray(a, n);
    printf("Elapsed time: %f seconds\n\n", elapsedTime);   // print the elapsed time
}

/* Runs a sort on a test case with known results */
void testsort(int a[], int n, int expected[]) {
    printf("Test case: ");              // print the test data
    printarray(a, n);
    sortarray(a, n);
    printf("Result: ");                 // print the result of sorting
    printarray(a, n);
    int ok = 1;                         // print whether the result is correct
    for (int i = 0; ok && (i < n); i++) {
        ok = (a[i] == expected[i]);
    }
    printf("%s\n\n", (ok ? "PASSED" : "FAILED"));
}

/* M A I N   F U N C T I O N */
int main(void) {
    char buffer[100];
    int nelements;
    int *randomdata;
    int *sorteddata;
    int *reversedata;

    srand(time(NULL));  // seed the random number generator

    // run test cases
    testsort(test1, n1, expected1);
    testsort(test2, n2, expected2);
    testsort(test3, n3, expected3);
    testsort(test4, n4, expected4);
    testsort(test5, n5, expected5);
    testsort(test6, n6, expected6);

    // run timed sorts
    int keepgoing = 1;
    while (keepgoing) {
        printf("How many elements? ");
        if (fgets(buffer, 100, stdin) == NULL) {
            keepgoing = 0;
        } else if (buffer[0] == '\n') {
            keepgoing = 0;
        } else {
            nelements = atoi(buffer);
            if (nelements <= 0) {
                printf("Must be a positive number of elements\n");
            } else {
                randomdata = genarray(nelements);
                if (randomdata != NULL) {
                    timedsort(randomdata, nelements);
                    free(randomdata);
                }
            }
        }
    }

    // run timed sorts on SORTED (already in order) data
    keepgoing = 1;
    while (keepgoing) {
        printf("How many elements (sorted data)? ");
        if (fgets(buffer, 100, stdin) == NULL) {
            keepgoing = 0;
        } else if (buffer[0] == '\n') {
            keepgoing = 0;
        } else {
            nelements = atoi(buffer);
            if (nelements <= 0) {
                printf("Must be a positive number of elements\n");
            } else {
                sorteddata = gensortedarray(nelements);
                if (sorteddata != NULL) {
                    timedsort(sorteddata, nelements);
                    free(sorteddata);
                }
            }
        }
    }

    // run timed sorts on REVERSE SORTED data
    keepgoing = 1;
    while (keepgoing) {
        printf("How many elements (reverse sorted data)? ");
        if (fgets(buffer, 100, stdin) == NULL) {
            keepgoing = 0;
        } else if (buffer[0] == '\n') {
            keepgoing = 0;
        } else {
            nelements = atoi(buffer);
            if (nelements <= 0) {
                printf("Must be a positive number of elements\n");
            } else {
                reversedata = genreversearray(nelements);
                if (reversedata != NULL) {
                    timedsort(reversedata, nelements);
                    free(reversedata);
                }
            }
        }
    }

    return 0;
}