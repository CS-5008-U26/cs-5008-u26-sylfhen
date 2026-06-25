/* Task 1A */
/*Generating Primes for 0.5 seconds*/
#include <stdio.h>
#include <time.h>
#include <math.h>

int main() {

    clock_t start = clock();
    /*double timeLimit = 0.5;*/

    long long int num = 2;/* Start checking for primes from 2 */
    long long int lastPrime = 2;/*intialize starts at 2 since 2 is the first prime*/

    while (1) {

        int isprime = 1;
        long long int limit = (long long int)sqrt((double)num);

        /*d is the divisor we are checking */
        for (long long int d = 2; d <= limit; d++) {
            if (num % d == 0) {
                isprime = 0;
                break;
            }
        }

        if (isprime) {

            lastPrime = num;

            clock_t now = clock();

            double elapsed =
                (double)(now - start) / CLOCKS_PER_SEC;

            if (elapsed >= 0.5)
                break;
        }

        num+2;
    }

    printf("Last prime found: %lld\n", lastPrime);

    return 0;
}