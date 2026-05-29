#include <stdio.h>

/* This programme calculates the complement of nums by removing the rightmost digit */
/* Recursively calling itself to complement the rest of nums           */
/* then composing that result with 9 - digit                           */

int FindingComplement(int nums) {
    if (nums == 0) {
        return 0;
    }
    if (nums < 0) {
        return -1;
    }
    return (FindingComplement(nums / 10) * 10) + (9 - (nums % 10));
}

int main() {
    char s[100];
    int nums;

    printf("Enter your number: ");
    if (scanf("%d", &nums) != 1) {
        printf("Invalid input\n");
        fgets(s, 100, stdin);   // pause before closing on invalid input
        return 1;
    }

    getchar();                  // consume the leftover '\n' from scanf

    if (nums < 0) {
        printf("Error: negative numbers are not supported\n");
    } else if (nums == 0) {
        printf("The complement is 9\n");
    } else {
        printf("The complement of %d is %d\n", nums, FindingComplement(nums));
    }

    printf("Press Enter to exit...\n");
    fgets(s, 100, stdin);       // now actually waits for Enter
    return 0;
}