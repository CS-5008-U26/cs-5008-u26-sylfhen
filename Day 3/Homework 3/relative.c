/* Program to compute the ET-function for integers from 2 to n */

/* Standard C Library */
#include <stdio.h>

/* Function to find the Greatest Common Divisor (GCD) */
int gcd(int p, int q) {
    while (q != 0) {
        int temp = q;
        q = p % q;
        p = temp;
    }
    return p;
}

/* The et-function: counts integers less than x that are relatively prime to x */
int et(int x) {
    int count = 0;
    for (int i = 1; i < x; i++) {
        if (gcd(x, i) == 1) {
            count++;
        }
    }
    return count;
}

int main() {
    char s[100];
    int n;

/* Prompt the user for input and read an integer. Handle invalid input gracefully. */   
    printf("Enter an integer n: ");
    if (scanf("%d", &n) != 1) {
        printf("Invalid input.\n");
        fgets(s, 100, stdin);
        return 1;
    }
    getchar();          /* consume leftover '\n' from scanf */

    /* et(x) is only meaningful for x >= 2 */
    if (n < 2) {
        printf("Error! n must be positive and atleast 2.\n");
        printf("Press Enter to exit...\n");
        fgets(s, 100, stdin);
        return 1;
    }
/* Print the results for et(x) from 2 to n */
    printf("\nResults for et(x) from 2 to %d:\n", n);
    printf("----------------------------\n");
    for (int i = 2; i <= n; i++) {
        printf("et(%3d) = %d\n", i, et(i));
    }

    printf("Press Enter to exit...\n");
    fgets(s, 100, stdin);
    return 0;
}
