/* This program asks for a number score from 0 to 100 and prints your letter grade */
/* Standard library for input and output */
#include <stdio.h>

/* Function to convert a numeric score (0-100) to a letter grade with +/- modifier */
/*Void handles all the printing internally */
void LetterGrade(int number) {
    /* Validate range */
    if (number < 0 || number > 100) {
        printf("Error! This is an Invalid Grade!\n");
        return;
    }

    /* Determining the letter grade */
    char letter;
    if (number <= 60) {
        letter = 'F';
    } else if (number <= 70) {
        letter = 'D';
    } else if (number <= 80) {
        letter = 'C';
    } else if (number <= 90) {
        letter = 'B';
    } else {
        letter = 'A';
    }

    /* Scores of F get no modifier */
    /*since passing grades only applies to (61-100)*/
    if (letter == 'F') {
        printf("Your grade is %c\n", letter);
        return;
    }

    /* Determine +/- modifier from last digit */
    int lastDigit = number % 10;
    char modifier;
    if (lastDigit >= 1 && lastDigit <= 3) {
        modifier = '-';
    /*lastDigit == 0 maps to + matching 8,9,0 */    
    } else if (lastDigit >= 8 || lastDigit == 0) {
        modifier = '+';
    } else {
        modifier = ' ';     /* digits 4-7: no modifier */
    }

    /* Print result */
    if (modifier == ' ') {
        printf("Your grade is %c\n", letter);
    } else {
        printf("Your grade is %c%c\n", letter, modifier);
    }
}

/* Main function to execute the program */
int main() {
    int number;
    printf("Enter the number score: ");
    scanf("%d", &number);

    LetterGrade(number);

    return 0;
}