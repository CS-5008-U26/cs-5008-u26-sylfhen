#include <stdio.h>

char LetterGrade(int number) {
    printf("Enter your grade: ");
    scanf("%d", &number);
    
    if(number < 1 || number > 10) {
        printf("Error! This is an Invalid Grade!");
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
    int number = 8;
    char grade = LetterGrade(number);
    printf("The grade is: %c\n", grade); // Outputs: B
    return 0;
}