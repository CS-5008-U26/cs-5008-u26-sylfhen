/*This program converts an integer to a Roman numeral*/

/*Standard library for input and output*/
#include <stdio.h>

void convertToRoman(int num) {
    int values[]    = {1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1};
    char *symbols[] = {"M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I"};

    /* Calculate the number of symbols in the arrays */
    int n = sizeof(values) / sizeof(values[0]);
    
    /* Print the Roman numeral */
    printf("The Roman Numeral:  ");
    for (int i = 0; i < n; i++) {
        while (num >= values[i]) {
            printf("%s", symbols[i]);
            num -= values[i];
        }
    }
    printf("\n");
}

/*Main function to handle user input and call conversion function*/

int main() {
    int number;
    printf("Enter a number between 1 and 4000:  ");
    if (scanf("%d", &number) != 1) {
        printf("Error: Please enter a valid integer.\n");
        return 1;
    }
    /* Validate the input number */
    /*after the out-of-range error the program exits immediately rather than falling through*/
    if (number < 1 || number > 4000) {
        printf("Error: Number must be between 1 and 4000.\n");
        return 1;
    }
    convertToRoman(number);
    return 0;
}