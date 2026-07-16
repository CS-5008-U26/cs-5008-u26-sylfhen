
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_TO_PRINT 100        // max number of elements to print
#define MAX_VALUE 100000000     // max value in randomly generated data

// Set to 1 for Task 1 (build heap top-down with heapifyDown)
// Set to 0 for Task 2 (build heap by calling heapifyUp on each element)
#define BUILD_HEAP_TOP_DOWN 1

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

/* H E A P   H E L P E R   F U N C T I O N S */

// swap - exchange two ints in an array
void swap(int a[], int i, int j) {
    int temp = a[i];
    a[i] = a[j];
    a[j] = temp;
}

/* heapifyDown assume the heap is valid everywhere */
void heapifyDown(int *a, int size, int index) {
    while (1) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int largest = i;

        if (left < size && a[left] > a[largest]) {
            largest = left;
        }
        if (right < size && a[right] > a[largest]) {
            largest = right;
        }

        if (largest == i) {
            // heap property already satisfied here, we're done
            break;
        }

        swap(a, i, largest);
        i = largest;   // continue sinking down
    }
}

/* heapifyUp assumes the heap is valid everywhere */
void heapifyUp(int a[], int i) {
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (a[i] <= a[parent]) {
            // heap property satisfied, we're done
            break;
        }
        swap(a, i, parent);
        i = parent;   // continue rising up
    }
}

/* Y O U R   S O R T   F U N C T I O N */

void sortarray (int a[], int n) {
#if BUILD_HEAP_TOP_DOWN
    
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapifyDown(a, n, i);
    }
#else
    
    for (int i = 1; i < n; i++) {
        heapifyUp(a, i);
    }
#endif
    for (int heapSize = n; heapSize > 1; heapSize--) {
        
        swap(a, 0, heapSize - 1);
        heapifyDown(a, heapSize - 1, 0);
    }
}

/* U T I L I T Y   F U N C T I O N S */
/* printarray - print the first MAX_TO_PRINT elements of an array
   a is the array, n is how many elements in the array
*/
void printarray(int a[], int n) {
    int numberToPrint = ((n < MAX_TO_PRINT) ? n : MAX_TO_PRINT);
    for (int i=0; i < numberToPrint; i++) {
        printf ("%d ", a[i]);
    }
    printf ("\n");
}


int *genarray(int numberofelements) {
    int *result = malloc (numberofelements * sizeof(int));
    if (result == NULL) {
        printf ("malloc failed");
    } else {
        // fill the result array with random numbers between 0 and MAX_VALUE
        for (int i=0; i<numberofelements; i++) {
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

// timedsort - runs a sort and records the elapsed time
// a is the array, n is how many elements

void timedsort (int a[], int n) {
    clock_t startTime = clock();        // get the start time
    sortarray (a, n);
    clock_t endTime = clock();          // get the end time
    double elapsedTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
    printf ("Result: ");                // print the sorted data
    printarray(a, n);
    printf ("Elapsed time: %f seconds\n\n", elapsedTime);   // print the elapsed time
}

// testsort - runs a sort on a test case with known results
// a is the array, n is how many elements
// expected is the array with the expected correct values after sorting

void testsort (int a[], int n, int expected[]) {
    printf ("Test case: ");             // print the test data
    printarray(a, n);
    sortarray(a, n);
    printf ("Result: ");                // print the result of sorting
    printarray(a, n);
    int ok = 1;                         // print whether the result is correct
    for (int i=0; ok && (i<n); i++) {
        ok = (a[i] == expected[i]);
    }
    printf ("%s\n\n", (ok ? "PASSED" : "FAILED"));
}

/* M A I N   F U N C T I O N */
void main () {
    char buffer[100];
    int nelements;
    int maxvalue;
    int *randomdata;

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
        printf ("How many elements? ");
        fgets(buffer, 100, stdin);
        if (buffer[0] == '\n') {
            keepgoing = 0;
        } else {
            nelements = atoi(buffer);
            if (nelements <= 0) {
                printf ("Must be a positive number of elements\n");
            } else {
                randomdata = genarray(nelements);
                if (randomdata != NULL) {
                    timedsort(randomdata, nelements);
                    free (randomdata);
                }
            }
        }
    }
}