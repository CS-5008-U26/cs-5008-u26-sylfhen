//This programme returns the highest Fibonacci number after 5 seconds:
#include <stdio.h>
#include <time.h>

// Fibonacci function
long long fib(int n) {
    if (n < 2)
        return 1;
    return fib(n - 1) + fib(n - 2);
}

int main() {
    //char s[100];

    //fgets(s, 100, stdin);

    clock_t start = clock();
    int n = 0;
    long long highest = 1;

    while (1) {
        double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
        if (elapsed >= 5.0)
            break;

        highest = fib(n);
        n++;
    }

    printf("Highest Fibonacci number reached in 5 seconds:\n");
    printf("F(%d) = %lld\n", n - 1, highest);

    return 0;
}