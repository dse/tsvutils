/**
 * tsvtable.c
 */

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>

#define LINESIZE 65536
#define TOKENLINES_INC 4096
#define LINESINC 4096
#define MAXCOLUMNS 256

char linebuf[LINESIZE];
char **lines = NULL;
size_t linebufsize = 0;
size_t numlines = 0;
size_t numcolumns = 0;
size_t columnwidth[MAXCOLUMNS];
size_t bytes = 0;
size_t alloced = 0;
char tokenbuf[LINESIZE];
char* tokens[MAXCOLUMNS];
size_t numtokens;

int main(int, char**);
int readfile(char*);
int readfp(FILE*, char*);
int chomp(char*);
size_t addline(char*);
size_t mbstrlen(const char* s);
void mbprintleftpad(size_t, char*);
void computecolumnwidths();
void printlines();
void tokenize(char* s);

#define printlocaleinfo(what) (printf("%s: %s\n", #what, setlocale(what, NULL)))

int
main(int argc, char** argv) {
    setlocale(LC_ALL, "");
    if (argc < 2) {
        readfp(stdin, "STDIN");
    } else {
        for (argc -= 1, argv += 1; argc && argv; argc -= 1, argv += 1) {
            readfile(*argv);
        }
    }
    computecolumnwidths();
    printlines();
}

int
readfile(char* filename) {
    FILE* fp;
    if ((fp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "tsvtable: cannot read %s: %s\n", filename, strerror(errno));
        return 0;
    }
    if (!readfp(fp, filename)) {
        return 0;
    }
    if (fp != stdin && fclose(fp) != 0) {
        fprintf(stderr, "tsvtable: cannot close %s: %s\n", filename, strerror(errno));
        return 0;
    }
    return 1;
}

int
readfp(FILE* fp, char* filename) {
    char* line;
    while ((line = fgets(linebuf, LINESIZE, fp)) != NULL) {
        bytes += strlen(line);
        if (addline(line) != 0) {
            if (isatty(fileno(stderr)) && numlines % 100 == 0) {
                fprintf(stderr, "  %zu lines; %zu bytes; %zu bytes alloced\r", numlines, bytes, alloced);
                fflush(stderr);
            }
        }
    }
    if (ferror(fp)) {
        fprintf(stderr, "tsvtable: error reading %s: %s\n", filename, strerror(errno));
    }
    if (isatty(fileno(stderr))) {
        fprintf(stderr, "  %zu lines; %zu bytes; %zu bytes alloced\n", numlines, bytes, alloced);
    }
    return 1;
}

int
chomp(char* line) {
    size_t len = strlen(line);
    if (len >= 2 && line[len - 2] == '\r' && line[len - 1] == '\n') {
        line[len - 2] = '\0';
        return 2;
    } else if (len >= 1 && line[len - 1] == '\n') {
        line[len - 1] = '\0';
        return 1;
    }
    return 0;
}

size_t
addline(char* line) {
    chomp(line);
    if (numlines >= linebufsize) {
        linebufsize += LINESINC;
        if (lines == NULL) {
            lines = (char **)malloc(sizeof(char *) * linebufsize);
            alloced += sizeof(char *) * LINESINC;
        } else {
            lines = (char **)realloc(lines, sizeof(char *) * linebufsize);
            alloced += sizeof(char *) * LINESINC;
        }
    }
    if (*line == '\0') {
        lines[numlines] = NULL;
        numlines += 1;
        return 0;
    }
    lines[numlines] = strdup(line);
    numlines += 1;
    alloced += strlen(line) + 1;
    return strlen(line);
}

void
computecolumnwidths() {
    size_t i, j;
    for (j = 0; j < MAXCOLUMNS; j += 1) {
        columnwidth[j] = 0;
    }
    for (i = 0; i < numlines; i += 1) {
        if (lines[i] == NULL) {
            continue;
        }
        tokenize(lines[i]);
        if (numcolumns < numtokens) {
            numcolumns = numtokens;
        }
        for (j = 0; j < numtokens; j += 1) {
            size_t len = mbstrlen(tokens[j]);
            if (columnwidth[j] < len) {
                columnwidth[j] = len;
            }
        }
    }
}

void
printlines() {
    size_t i;
    size_t j;
    for (j = 0; j < numcolumns; j += 1) {
        printf("%zu", columnwidth[j]);
        if (j < (numcolumns - 1)) {
            fputs(", ", stdout);
        } else {
            fputs("\n", stdout);
        }
    }
    for (i = 0; i < numlines; i += 1) {
        if (lines[i] == NULL) {
            numtokens = 0;
        } else {
            tokenize(lines[i]);
        }
        for (j = 0; j < numcolumns; j += 1) {
            mbprintleftpad(columnwidth[j], j < numtokens ? tokens[j] : "");
            if (j < (numcolumns - 1)) {
                fputs(" | ", stdout);
            } else {
                fputs("\n", stdout);
            }
        }
    }
}

void
tokenize(char* s) {
    strcpy(tokenbuf, s);
    char* p;
    numtokens = 0;
    size_t j;
    for (j = 0, p = tokenbuf; j < MAXCOLUMNS; j += 1) {
        if ((tokens[j] = strsep(&p, "\t")) == NULL) {
            break;
        }
        numtokens += 1;
    }
}

size_t
mbstrlen(const char* s) {
    if (MB_CUR_MAX == 1) {
        return strlen(s);
    }
    size_t result = mbstowcs(NULL, s, MB_CUR_MAX);
    if (result == (size_t)-1) {
        return strlen(s);
    }
    if (result == (size_t)-2) {
        return strlen(s);
    }
    return result;
}

void
mbprintleftpad(size_t len, char* s) {
    if (MB_CUR_MAX == 1) {
        printf("%-*s", (int)len, s);
        return;
    }
    fputs(s, stdout);
    size_t rem = len - mbstrlen(s);
    printf("%-*s", (int)rem, "");
}
