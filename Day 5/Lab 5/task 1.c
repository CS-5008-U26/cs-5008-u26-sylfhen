/*Standard Libraries */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void condition(char *in, char *out) {
    int i = 0;
    int j = 0;

    while (*(in + i) != '\0' && *(in + i) != '\n') {
        if (isalpha(*(in + i))) {
            out[j++] = tolower(*(in + i));
        }
        i++;
    }
    out[j] = '\0';
}

// Function to check if a string is a palindrome
int palindrome(char *in) {
    char conditioned[200];
    condition(in, conditioned);
    int left = 0;
    int right = (int)strlen(conditioned) - 1;

    while (left < right) {
        if (conditioned[left] != conditioned[right]) {
            return 0; // Not a palindrome
        }
        left++;
        right--;
    }
    return 1; // Is a palindrome
}



int main() {
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
        printf("%d\n", (int)strlen(inputBuffer)- 1);
    }
   } 
  }

