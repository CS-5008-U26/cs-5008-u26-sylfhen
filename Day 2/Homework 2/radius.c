/* This program accepts an integer radius and prints an integer approximation of the circle's area */

/*Library Declarations*/
#include <stdio.h>
#include <math.h>

int main() {
    int radius;
    double pi = acos(-1);//the value of pi calculated in C

    /* Prompt and validate integer radius */
    printf("Enter an integer radius: ");
    if (scanf("%d", &radius) != 1) {
        printf("Invalid input. Please enter an integer.\n");
        return 1;
    }

    /* Calculate area to nearest integer */
    int area = (int)(pi * radius * radius);

    /* Print result matching expected format */
    printf("For a circle of radius %d.0 the area is %d\n", radius, area);

    return 0;
}