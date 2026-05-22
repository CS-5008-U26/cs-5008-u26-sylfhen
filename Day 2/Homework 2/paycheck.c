/*This program calculates the wages of an employee based on their hourly rate and hours worked, including overtime pay for hours worked over 40.*/

/*Standard library for input and output*/
#include <stdio.h>

/* Global constant representing the standard work week threshold.
   Hours worked beyond this will be paid at the overtime rate. */
const float WeeklyHours = 40.0;

float wages(float hourlyRate, float hours) {
    if (hours <= WeeklyHours) {
        return (hourlyRate * hours);
    } else {
        /* Overtime: pay the first 40 hours normally, then 1.5x for the remainder */
        return (hourlyRate * WeeklyHours) + (hourlyRate * 1.5 * (hours - WeeklyHours));
    }
}

int main() {
    float hourlyRate, hours;

    printf("Enter hourly rate: ");
    scanf("%f", &hourlyRate);

    printf("Enter number of hours: ");
    scanf("%f", &hours);

    printf("This is how much you are due to be paid: $%.2f\n", wages(hourlyRate, hours));
    return 0;
}