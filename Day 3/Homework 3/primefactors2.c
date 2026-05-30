/*This programme asks the User for a number and prints the Prime factors of said number recursively*/
/*Standard C Library*/
#include <stdio.h>

/* Function declaration */
void RecursivePrimeFactors(long long num, long long divisor, int first);

/*
 * Recursively finds and prints the prime factorisation of num.
 * Each call prints one prime factor, then recurses for the rest.
 * divisor: current candidate factor to test*/
 
void RecursivePrimeFactors(long long num, long long divisor, int first) {
    /* Base case: num fully factored */
    if (num == 1) {
        return;
    }

    /* If divisor^2 > num, then num itself is a remaining prime factor */
    if ((divisor * divisor) > num) {
        if (!first) {
            printf(" * ");
        }
        printf("%lld", num);
        return;
    }

    /* Recursive case: if divisor divides num, print it and recurse on num/divisor */
    if (num % divisor == 0) {
        if (!first) {
            printf(" * ");
        }
        printf("%lld", divisor);
        RecursivePrimeFactors((num / divisor), divisor, 0);
    } else {
        /* divisor does not divide num, try the next candidate */
        RecursivePrimeFactors(num, (divisor + 1), first);
    }
}

/* Wrapper: prints the header line, then kicks off the recursion */
void primeFactors(long long num) {
    printf("The prime factors of %lld: ", num);
    RecursivePrimeFactors(num, 2, 1);
    printf("\n");
}

int main() {
    char s[100];
    long long num;

    /* Prompt the user for input and read an integer. Handle invalid input gracefully. */
    printf("Enter a number: ");
    if (scanf("%lld", &num) != 1) {
        printf("Invalid input\n");
        fgets(s, 100, stdin);   /* pause before closing on invalid input */
        return 1;
    }
    getchar();                  /* consume leftover '\n' from scanf */

    /* Capturing edge cases where numbers do not have prime factorisations */
    /* Keep in mind we are only considering positive whole integers greater than 1 */
    if (num < 0) {
        printf("Error: negative numbers do not have prime factorisations\n");
    } else if (num == 0 || num == 1) {
        printf("Error: %lld has no prime factorisation\n", num);
    } else {
        primeFactors(num);
    }

    printf("Press Enter to exit...\n");
    fgets(s, 100, stdin);       /* waits for Enter before closing */
    return 0;
}