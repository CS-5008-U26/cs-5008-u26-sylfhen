/*Standard Libraries*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*Removing the trailing newline character from the input string*/
void killNewline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

/*Extract the next comma separated field from 'start' into 'out'*/
char *getNextField(char *start, char separator, char *out) {
    if(*start == '\0') {
        out[0] = '\0';
        return start;
    }
    char *sepPos = strchr(start, separator);
    if (sepPos == NULL) {
        strcpy(out, start); // No separator found, copy the rest of the string
    
    /* Strip double-quotes if present */
        int outLen = strlen(out);
        if (outLen >= 2 && out[0] == '"' && out[outLen - 1] == '"') {
            out[outLen - 1] = '\0';
            memmove(out, out + 1, outLen - 1);
        }
        return start + strlen(start); // Return pointer to the end of the string
        
    } else {
        size_t fieldLength = sepPos - start;
        strncpy(out, start, fieldLength);
        out[fieldLength] = '\0'; // Null-terminate the output string
    }

    /* Strip double-quotes if present */
        int outLen = strlen(out);
        if (outLen >= 2 && out[0] == '"' && out[outLen - 1] == '"') {
            out[outLen - 1] = '\0';
            memmove(out, out + 1, outLen - 1);
        }
        return sepPos + 1; // Return pointer to the character after the separator
    }

int main(void) {
    char filename[1000];
    char buffer[1000];
    char field[1000];

    printf("Enter the filename: ");
    fgets(filename, 1000, stdin);
    killNewline(filename);

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return 1;
    } else {
        while (fgets(buffer, 1000, file) != NULL) {
            killNewline(buffer);
            
            printf(">%s<\n", buffer);

            char *ptr = buffer;
            while (*ptr != '\0') {
                ptr = getNextField(ptr, ',', field);
                if (*ptr != '\0') {
                    printf("   >%s<\n", field);
                } else {
                    printf("   >%s<\n", field);
                    break;
                }
            }
        }
    }

    fclose(file);

    return 0;
}