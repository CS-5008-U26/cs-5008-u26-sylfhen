#include <stdio.h>
#include <string.h> // for string functions like strcspn


int main() {
    char s[100];
    char t[100];
    printf ("What is your name? ");
    scanf("%s", s);
    fgets(s,100,stdin);
    printf("%s? That's a funny name!",t);
}
