/* printCsv - print a CSV file on the console
*/

#include <stdio.h>
#include <string.h>

void killNewline (char *str) {
    char *next = str;
    while (*next != '\0') {
        next++;
    }
    // next is now pointing to the end of string null
    if (next != str) {
        if (*(next-1) == '\n') {
            *(next-1) = '\0';
        }
    }
}

// remove outer double-quotes from str, leaving the result in str
void killDoubleQuotes (char *str) {
    if ((*str == '"') && (*(str+strlen(str)-1) == '"')) {
        // we have double-quotes at the start and end
        // kill double-quote at the far end
        *(str+strlen(str)-1) = '\0';
        // kill double-quote at the start of the string
        char *p = str+1;
        while (*p) {
            *(p-1) = *p;
            p++;
        }
        *(p-1) = '\0';
    }
}

// return value is a pointer to the next character to process
// when the last field is processed, return a pointer to end-of-string
char *getNextField (char *start, char separator, char *out) {
    if (*start == '\0')
    {
        return (NULL);
    }
    char *comma = strchr(start, ',');
    if (comma == NULL) {
        // there are no commas in the string, just copy it to the output
        strcpy(out, start);
        return (start + strlen(start));
    } else {
        int numberOfCharacters = comma - start;
        strncpy (out, start, numberOfCharacters);
        out[numberOfCharacters] = '\0';
        return (comma+1);
    }
}

main () {
    char s[100];
    char filename[200];
    char myBuffer[1000];
    char nextField[1000];

    printf ("Enter a string: ");
    fgets(myBuffer, 1000, stdin);
    killNewline (myBuffer);
    killDoubleQuotes (myBuffer);
    printf ("Result: %s\n", myBuffer);
    fgets(s,100,stdin);
    exit(0);

    // get the filename
    printf ("Enter the filename: ");
    fgets(filename, 200, stdin);
    killNewline (filename);

    // process the CSV file
    FILE *f = fopen (filename, "r");
    if (f == NULL) {
        printf ("ERROR: Can't open file %s\n", filename);
    } else {
        int keepReading = 1;
        while (keepReading) {
            char *whatWasRead = fgets (myBuffer, 1000, f);
            keepReading = (whatWasRead != NULL);
            if (keepReading) {
                // process the line in myBuffer
                killNewline (myBuffer);
                printf (">%s<\n", myBuffer);
                char *p = myBuffer;
                while (p != NULL) {
                    p = getNextField (p, ',', nextField);
                    if (p != NULL) {
                        // process the next field
                        printf ("   >%s<\n", nextField);
                    }
                }
            }
        }
        fclose(f);
    }

    // pause for the console to be examined
    fgets(s,100,stdin);
}