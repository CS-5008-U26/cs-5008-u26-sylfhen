/* This program asks for cents (1-100) and prints the minimum coins needed */
#include <stdio.h>

int main() {
    int cents;

    /* Prompt and validate input */
    printf("Enter a number of cents: ");
    scanf("%d", &cents);

    if (cents < 1 || cents > 100) {
        printf("Invalid input. Please enter an amount between 1 and 100.\n");
        return 1;
    }

    /* Calculate coins using greedy approach */
    int quarters = cents / 25;
    cents %= 25;

    int dimes = cents / 10;
    cents %= 10;

    int nickels = cents / 5;
    cents %= 5;

    int pennies = cents;

    /* Count how many coin types are needed */
    int needed = 0;

    if (quarters > 0) needed++;
    if (dimes > 0)    needed++;
    if (nickels > 0)  needed++;
    if (pennies > 0)  needed++;

    printf("That requires ");

    int printed = 0;

    /* Quarters */
    if (quarters > 0) {
        printed++;

        printf("%d %s",
               quarters,
               (quarters == 1) ? "quarter" : "quarters");

        if (printed < needed - 1)
            printf(", ");
        else if (printed == needed - 1)
            printf(" and ");
    }

    /* Dimes */
    if (dimes > 0) {
        printed++;

        printf("%d %s",
               dimes,
               (dimes == 1) ? "dime" : "dimes");

        if (printed < needed - 1)
            printf(", ");
        else if (printed == needed - 1)
            printf(" and ");
    }

    /* Nickels */
    if (nickels > 0) {
        printed++;

        printf("%d %s",
               nickels,
               (nickels == 1) ? "nickel" : "nickels");

        if (printed < needed - 1)
            printf(", ");
        else if (printed == needed - 1)
            printf(" and ");
    }

    /* Pennies */
    if (pennies > 0) {
        printed++;

        printf("%d %s",
               pennies,
               (pennies == 1) ? "penny" : "pennies");
    }

    printf(".\n");

    return 0;
}