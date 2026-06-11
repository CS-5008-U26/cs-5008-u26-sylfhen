/*Standard Libraries */
#include <stdio.h>
#include <string.h>
#include <ctype.h>


void main() {
char inputBuffer [200];

inputBuffer[0] = 'X';        // so we will do the loop at least once

while (inputBuffer[0] != '\n') {

// print "Enter a string: " onto the console
    printf("Enter a string: ");
// read one line from the console into inputBuffer
    fgets(inputBuffer, sizeof(inputBuffer), stdin);

    if (inputBuffer[0] != '\n') {

// print inputBuffer onto the console
        printf("%s", inputBuffer);
        printf("%d", (int)strlen(inputBuffer));

}

}

}
   