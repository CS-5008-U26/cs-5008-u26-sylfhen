
/*This programme asks the User for a number and prints the Primefactors of said number*/
/*Standard C Library*/
#include <stdio.h>

/* Function declaration */
void primeFactors(long long num);

/* Finds and prints the prime factorisation of num */
void primeFactors(long long num) {
    int first = 1;
    long long originalNum = num;   // save original number before num gets divided down

    printf("The prime factors of %lld are ", originalNum);  //passing originalNum as argument

    /* Loop through to find prime factors starting from 2 */
    for (long long i = 2; i * i <= num; i++) {
        while (num % i == 0) {
            if (!first) {
                printf(" * ");
            }
            printf("%lld", i);
            first = 0;
            num /= i;
        }
    }

    /* If num is still greater than 1, the remaining part is prime */
    if (num > 1) {
        if (!first) {
            printf(" * ");
        }
        printf("%lld", num);
    }

    printf("\n");
}

int main() {
    char s[100];
    long long num;

    printf("Enter a number: ");
    if (scanf("%lld", &num) != 1) {
        printf("Invalid input\n");
        fgets(s, 100, stdin);   // pause before closing on invalid input
        return 1;
    }

    getchar();                  // consume leftover '\n' from scanf

    /* Numbers <= 1 do not have prime factorisations */
    if (num < 0) {
        printf("Error: negative numbers are not supported\n");
    } else if (num == 0 || num == 1) {
        printf("Error: %lld has no prime factorisation\n", num);
    } else {
        primeFactors(num);
    }

    printf("Press Enter to exit...\n");
    fgets(s, 100, stdin);       // waits for Enter before closing
    return 0;
}