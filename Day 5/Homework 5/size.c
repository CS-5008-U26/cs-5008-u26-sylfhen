/*
 * size.c
 *
 * Prints the size of a file in bytes.
 * Accepts the filepath as a command line argument,
 * or prompts the user for a filepath*/ 

/* Standard Libraries */
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

/* Remove the trailing newline character from the input string */
void killNewline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

int main(int argc, char *argv[]) {
    char filename[1000];

    /*If a filepath is provided as a command line argument, use it */
    if (argc > 1) {
        /* Use the command line argument as the filepath */
        /*argv[0] is the program name, argv[1] is the first argument*/
        strncpy(filename, argv[1], 1000);
    } else {
        /* Prompt the user for a filepath */
        printf("Enter the filepath: ");
        fgets(filename, 1000, stdin);
        killNewline(filename);
    }

    /* Get file information using stat */
    struct stat fileInfo;
    if (stat(filename, &fileInfo) != 0) {
        printf("Error accessing the filepath: %s\n", filename);
        return 1;
    }

    /* Print the size of the file in bytes */
    printf("Size of the file '%s': %lld bytes\n", filename, (long long)fileInfo.st_size);

    return 0;
}