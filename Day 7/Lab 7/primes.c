/* Task 1A */
/*Generating Primes for 0.5 seconds*/
#include <stdio.h>
#include <time.h>
#include <math.h>

int main() {

    clock_t start = clock();

    long long int num = 2;/* Start checking for primes from 2 */
    long long int lastPrime = 2;/*intialize starts at 2 since 2 is the first prime*/

    while (1) {

        int prime = 1;
        long long int timelimit = (long long int)sqrt((double)num);

        /*d is the divisor we are checking */
        for (long long int d = 2; d <= timelimit; d++) {
            if (num % d == 0) {
                prime = 0;
                break;
            }
        }

        if (prime) {

            lastPrime = num;

            clock_t now = clock();

            double elapsed =
                (double)(now - start) / CLOCKS_PER_SEC;

            if (elapsed >= 0.5)
                break;
        }

        num++;
    }

    printf("Last prime found: %lld\n", lastPrime);

    return 0;
}