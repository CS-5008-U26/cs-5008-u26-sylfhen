#include <stdio.h>
#include <string.h> // for string functions like strcspn


int main() {
    char s[100]; 

    /* excluded char t[100] since it is not used elsewhere in the code */
    
    printf ("What is your name? ");
    scanf("%s", s);
    fgets(s,100,stdin);
    printf("%s? That's a funny name!",s);
}
