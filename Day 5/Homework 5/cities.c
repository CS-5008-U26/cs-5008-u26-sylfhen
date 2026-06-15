/* 
 * cities.c
 *
 * Getting data about US cities from a CSV file.
 *
/* Standard Libraries */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Remove the trailing newline character from the input string */
void killNewline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

/* Extract the next comma-separated field from 'start' into 'out' */
char *getNextField(char *start, char separator, char *out) {
    /* No more fields */
    if (*start == '\0') {
        out[0] = '\0';
        return start;
    }
/* Find the position of the next separator */
    char *sepPos = strchr(start, separator);

    if (sepPos == NULL) {
        /* No separator found copy the rest of the string */
        strcpy(out, start);

        /* Strip double-quotes if present */
        int outLen = strlen(out);
        if (outLen >= 2 && out[0] == '"' && out[outLen - 1] == '"') {
            out[outLen - 1] = '\0';
            memmove(out, out + 1, outLen - 1);
        }

        return start + strlen(start);

    } else {
        /* Copy characters up to the separator */
        size_t fieldLength = sepPos - start;
        strncpy(out, start, fieldLength);
        out[fieldLength] = '\0';

        /* Strip double-quotes if present */
        int outLen = strlen(out);
        if (outLen >= 2 && out[0] == '"' && out[outLen - 1] == '"') {
            out[outLen - 1] = '\0';
            memmove(out, out + 1, outLen - 1);
        }

        return sepPos + 1;
    }
}

int main(void) {
    char filename[1000];
    char buffer[1000];
    char field[1000];

    printf("Enter the filename: ");
    fgets(filename, 1000, stdin);
    killNewline(filename);

/* Open the specified file for reading */    
    FILE *file = fopen(filename, "r");

/* Check if the file was opened successfully */    
    if (file == NULL) {
        printf("Error opening the file: %s\n", filename);
        return 1;
    }

    /*Throw away the first line (column headers)*/
    fgets(buffer, 1000, file);

    /*initializing the required fields*/
    long populationTotal = 0;
    double latitude = -999.0;
    char mostNorthernCity[1000];
    mostNorthernCity[0] = '\0';

    /*Process the next twenty (20) lines*/
    int cityCount = 0;
    while (cityCount < 20 && fgets(buffer, 1000, file) != NULL) {
        killNewline(buffer);

        char *ptr = buffer;
        int fieldIndex = 1;

        char cityName[1000];
        double cityLatitude = 0.0;
        int cityPopulation = 0;

        /*Walk through each field, picking out columns 2, 7, and 9 */
        while (*ptr != '\0') {
            ptr = getNextField(ptr, ',', field);

            if (fieldIndex == 2) {
                /*ASCII city name*/
                strncpy(cityName, field, 1000);
            } else if (fieldIndex == 7) {
                /*Latitude*/
                cityLatitude = atof(field);
            } else if (fieldIndex == 9) {
                /*Population*/
                cityPopulation = atoi(field);
            }

            fieldIndex++;
        }
        /* Debug: print each city's data */
        printf("City: %s | Lat: %f | Pop: %d\n", cityName, cityLatitude, cityPopulation);

        /*Update the total population*/
        populationTotal += cityPopulation;

        /*Check if this city is the most northern*/
        if (cityLatitude > latitude) {
            latitude = cityLatitude;
            strncpy(mostNorthernCity, cityName, 1000);
        }

        cityCount++;
    }

    /*Close the file*/
    fclose(file);

    /*Print the results*/
    printf("Total population: %ld\n", populationTotal);
    printf("Most northern city is: %s\n", mostNorthernCity);

    return 0;
}