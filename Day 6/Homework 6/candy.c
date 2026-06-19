/*
 * candy.c
 *
 * Reading Halloween Candy Rankings from a CSV file.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* Struct to store candy's data (all on the heap via pointer) */
typedef struct reading_struct {
    char *competitorname;
    int chocolate;
    int fruity;
    int caramel;
    int peanutalmondy;
    int nougat;
    int crispedricewafer;
    int hard;
    int bar;
    int pluribus;
    double sugarpercent;
    double pricepercent;
    double winpercent;
} Candy;

/* Remove the trailing newline character from the input string */
void killNewline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

/* Extract the next comma-separated field from 'start' into 'out' */
char *getNextField(char *start, char separator, char *out) {
    if (*start == '\0') {
        out[0] = '\0';
        return start;
    }
    char *sepPos = strchr(start, separator);
    if (sepPos == NULL) {
        strcpy(out, start);
        int outLen = strlen(out);
        if (outLen >= 2 && out[0] == '"' && out[outLen - 1] == '"') {
            out[outLen - 1] = '\0';
            memmove(out, out + 1, outLen - 1);
        }
        return start + strlen(start);
    } else {
        size_t fieldLength = sepPos - start;
        strncpy(out, start, fieldLength);
        out[fieldLength] = '\0';
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

    fgets(filename, 1000, stdin);
    killNewline(filename);

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening the filepath: %s\n", filename);
        return 1;
    }

    fgets(buffer, 1000, file);

    int capacity = 100;
    int count = 0;
    Candy **candies = malloc(capacity * sizeof(Candy *));
    if (candies == NULL) {
        printf("Memory allocation failed.\n");
        fclose(file);
        return 1;
    }

    while (fgets(buffer, 1000, file) != NULL) {
        killNewline(buffer);

        Candy *c = malloc(sizeof(Candy));
        if (c == NULL) {
            printf("Memory allocation failed.\n");
            break;
        }

        char *ptr = buffer;
        int fieldIndex = 1;

        while (*ptr != '\0') {
            ptr = getNextField(ptr, ',', field);
            switch (fieldIndex) {
                case 1:
                    c->competitorname = malloc(strlen(field) + 1);
                    if (c->competitorname == NULL) {
                        free(c); c = NULL; break;
                    }
                    strcpy(c->competitorname, field);
                    break;
                case 2:  c->chocolate        = atoi(field); break;
                case 3:  c->fruity           = atoi(field); break;
                case 4:  c->caramel          = atoi(field); break;
                case 5:  c->peanutalmondy    = atoi(field); break;
                case 6:  c->nougat           = atoi(field); break;
                case 7:  c->crispedricewafer = atoi(field); break;
                case 8:  c->hard             = atoi(field); break;
                case 9:  c->bar              = atoi(field); break;
                case 10: c->pluribus         = atoi(field); break;
                case 11: c->sugarpercent     = atof(field); break;
                case 12: c->pricepercent     = atof(field); break;
                case 13: c->winpercent       = atof(field); break;
            }
            fieldIndex++;
        }

        if (c == NULL) continue;

        if (count == capacity) {
            capacity *= 2;
            Candy **tmp = realloc(candies, capacity * sizeof(Candy *));
            if (tmp == NULL) {
                printf("Memory allocation failed.\n");
                free(c->competitorname);
                free(c);
                break;
            }
            candies = tmp;
        }
        candies[count++] = c;
    }

    fclose(file);

    /* ── Task 1: Print all candy names ── */
    printf("All candies (%d total):\n", count);
    for (int i = 0; i < count; i++) {
        printf("%s\n", candies[i]->competitorname);
    }

    /* ── Task 2: Chocolate candies ── */
    printf("\nChocolate candies (UPPER = has caramel, lower = no caramel):\n");
    int chocCount   = 0;
    int chocCaramel = 0;

    for (int i = 0; i < count; i++) {
        if (!candies[i]->chocolate) continue;
        chocCount++;

        /* Copy name into a local buffer to avoid modifying heap string */
        char nameCopy[1000];
        strncpy(nameCopy, candies[i]->competitorname, 999);
        nameCopy[999] = '\0';

        if (candies[i]->caramel) {
            chocCaramel++;
            for (int j = 0; nameCopy[j]; j++)
                nameCopy[j] = toupper((unsigned char)nameCopy[j]);
        } else {
            for (int j = 0; nameCopy[j]; j++)
                nameCopy[j] = tolower((unsigned char)nameCopy[j]);
        }
        printf("%s\n", nameCopy);
    }

    printf("Percent of chocolate candies that also have caramel: %.2f%%\n",
           100.0 * chocCaramel / chocCount);

    /* ── Free all heap memory ── */
    for (int i = 0; i < count; i++) {
        free(candies[i]->competitorname);
        free(candies[i]);
    }
    free(candies);

    return 0;
}