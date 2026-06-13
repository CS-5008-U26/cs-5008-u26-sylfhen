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
void getNextField(char *start, char separator, char *out) {
    if(*start == '\0') {
        return NULL;
    }
    char *sepPos = strchr(start, separator);
    if (sepPos == NULL) {
        strcpy(out, start); // No separator found, copy the rest of the string
    } else {
        size_t fieldLength = sepPos - start;
        strncpy(out, start, fieldLength);
        out[fieldLength] = '\0'; // Null-terminate the output string
    }



int main()
{
    char s[100];
    
    fgets(s, 100, stdin);
    killNewline(s);

    return 0;
}