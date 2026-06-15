/* 
 * printCsv.c
 *
 * Reads a CSV file and prints each line and its fields.
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

    /* Read each line from the file */
    while (fgets(buffer, 1000, file) != NULL) {
        killNewline(buffer);

        /* Print the full line */
        printf(">%s<\n", buffer);

        /* Print each field */
        char *ptr = buffer;
        while (*ptr != '\0') {
            ptr = getNextField(ptr, ',', field);
            printf("   >%s<\n", field);
        }
    }

    fclose(file);
    return 0;
}