/* This program asks for a number score from 1 to 10 and prints your letter grade */
/* Standard library for input and output */
#include <stdio.h>

/* Function to convert a numeric grade to a letter grade using switch */
char LetterGrade(int number) {
    if (number < 1 || number > 10) {
        printf("Error! This is an Invalid Grade!\n");
        return 'I';
    }
/* Convert numeric grade to letter grade using switch statement */
/* Cases are grouped to handle ranges of numeric grades that correspond to the same letter grade */
    switch (number) {
        case 10:
        case 9:
            return 'A';
        case 8:
            return 'B';
        case 7:
            return 'C';
        case 6:
            return 'D';
        case 5:
        case 4:
            return 'E';
        default:
            return 'F';
    }
}

/* Main function to execute the program */
int main() {
    int number;
    printf("Enter the number score: ");
    scanf("%d", &number);           // input collected here, once

    char grade = LetterGrade(number);

    if (grade != 'I') {
        printf("Your grade is %c\n", grade);
    }

    return 0;
}