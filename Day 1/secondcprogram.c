#include <stdio.h>
#include <string.h> // for string functions like strcspn


int main() {
    char s[100]; 

    /* excluded char t[100] since it is not used elsewhere in the code */
    
    printf ("What is your name? ");
    /* Read a line of input from the user and store it in the string s 
    fgets is more robust than scanf for reading strings with spaces */
    fgets(s,100,stdin);
    
    /* Remove the newline character that fgets stores */
    s[strcspn(s, "\n")] = 0;
    
    printf("%s? That's a funny name!",s);
    return 0;
}
