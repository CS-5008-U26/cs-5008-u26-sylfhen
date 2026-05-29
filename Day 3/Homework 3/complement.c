/*Standard input and output Header*/
#include <stdio.h>

/* This programme calculates the complement of nums by removing the rightmost digit */
/* Recursively calling itself to complement the rest of nums           */
/* then composing that result with 9 - digit                           */

/*Counting the length of nums to prevent overflow*/ 
int countDigits(long long nums) {
    int count = 0;
    while (nums != 0) {
        nums /= 10;
        count++;
    }
    return count;
}


/* Base Case: If nums is 0, the complement is 0. */
/* If nums is negative, the function returns -1 to indicate an error. */
long long FindingComplement(long long nums) {
    if (nums == 0) {
        return 0;
    }
    if (nums < 0) {
        return -1;
    }
 /*Recursive Case: Remove the rightmost digit, find the complement of the rest, and combine with 9 - digit*/   
    return (FindingComplement(nums / 10) * 10) + (9 - (nums % 10));
}

int main() {
    char s[100];
    long long nums;
/* Prompt the user for input and read an integer. Handle invalid input gracefully. */
    printf("Enter your number: ");
    if (scanf("%lld", &nums) != 1) {
        printf("Invalid input\n");
        fgets(s, 100, stdin);   // pause before closing on invalid input
        return 1;
    }

    getchar();                  // consume the leftover '\n' from scanf

    /* Handle negative numbers and zero as special cases */
    if (nums < 0) {
        printf("Error: negative numbers are not included\n");
    } else if (nums == 0) {
        printf("The complement of 0 is 9\n");
    } else {
        long long result = FindingComplement(nums);
        int numDigits = countDigits(nums);
        int resultDigits = countDigits(result);
        
         /*Print any leading zeros if necessary*/   
        printf("The complement of %lld is ", nums);
        for (int i = 0; i < numDigits - resultDigits; i++) {
            printf("0");
        }
        printf("%lld\n", result);
    }
    printf("Press Enter to exit...\n");
    fgets(s, 100, stdin);       // now actually waits for Enter
    return 0;
}