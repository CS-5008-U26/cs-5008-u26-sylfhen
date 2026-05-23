#include <stdio.h>

char LetterGrade(int number) {
    if (number < 1 || number > 10) {
        printf("Error! This is an Invalid Grade!\n");
        return 'I';          // defined return value for the error case
    } else if (number >= 9) {
        return 'A';
    } else if (number >= 8) {
        return 'B';
    } else if (number >= 7) {
        return 'C';
    } else if (number >= 6) {
        return 'D';
    } else if (number >= 4) {
        return 'E';
    } else {
        return 'F';
    }
}

int main() {
    int number;
    printf("Enter your grade: ");
    scanf("%d", &number);           // input collected here, once

    char grade = LetterGrade(number);

    if (grade != 'I') {
        printf("The grade is: %c\n", grade);
    }

    return 0;
}