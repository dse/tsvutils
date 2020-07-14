/**
 * tsvtable.c
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

/* max line width */
#define LINESIZE 65536

/* max number of tab-separated fields */
#define MAXTOKENS 256

/* number of lines to allocate memory for at each step */
#define TOKENLINES_INC 4096

/* each line of data read */
char* linebuf;

/* each array of tokens */
char** tokens;

/* total number of columns */
size_t numcolumns;

/* width of each column */
size_t* columnwidth;

/* lifetime array of arrays of tokens */
char*** tokenlines;

/* each line's number of tokens */
size_t* tokenlines_numtokens;

/* lifetime number of lines */
size_t numlines;

/* lifetime number of lines allocated */
size_t tokenlines_bufsize;

int main(int, char**);
int readfile(char*);
int readfp(FILE*, char*);
int chomp(char*);
size_t addline(char*);
size_t addtokens(size_t, char**);
void init();
size_t getcolumnwidths(size_t, char***);
void printcolumns(size_t, char***);

int
main(int argc, char** argv) {
    init();
    if (argc < 2) {
        readfp(stdin, "STDIN");
    } else {
        for (argc -= 1, argv += 1; argc && argv; argc -= 1, argv += 1) {
            readfile(*argv);
        }
    }
    getcolumnwidths(numlines, tokenlines);
    printcolumns(numlines, tokenlines);
}

void
init() {
    tokenlines = NULL;
    tokenlines_numtokens = NULL;
    numlines = 0;
    tokenlines_bufsize = 0;
    linebuf = NULL;
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
    if (!fclose(fp)) {
        fprintf(stderr, "tsvtable: cannot close %s: %s\n", filename, strerror(errno));
        return 0;
    }
    return 1;
}

int
readfp(FILE* fp, char* filename) {
    char* line;
    while (1) {
        if (feof(fp)) {
            return 1;
        }
        linebuf = (char*)malloc(sizeof(char *) * LINESIZE);
        if ((line = fgets(linebuf, LINESIZE, fp)) == NULL) {
            if (errno == 0) {
                return 1;
            }
            free(linebuf);
            fprintf(stderr, "tsvtable: fgets error from %s: %s (%d)\n", filename, strerror(errno), errno);
            return 0;
        }
        if (addline(line) == 0) {
            free(linebuf);
        }
    }
    return 1;
}

#define ISCRLF(c) ((c == '\r' || c == '\n'))

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
    char* token;
    char* stringp;
    size_t j;
    chomp(line);
    if (*line == '\0') {
        return addtokens(0, NULL);
    }
    tokens = (char**)malloc(sizeof(char *) * MAXTOKENS);
    size_t numtokens;
    for (j = 0, numtokens = 0, stringp = line; j < MAXTOKENS; j += 1) {
        token = strsep(&stringp, "\t");
        if (token == NULL) {
            break;
        }
        tokens[j] = token;
        numtokens += 1;
    }
    return addtokens(numtokens, tokens);
}

size_t
addtokens(size_t numtokens, char** tokens) {
    if (numlines >= tokenlines_bufsize) {
        if (tokenlines_bufsize == 0) {
            tokenlines_bufsize += TOKENLINES_INC;
            tokenlines           = (char ***)malloc(sizeof(char **) * tokenlines_bufsize);
            tokenlines_numtokens = (size_t*)malloc(sizeof(size_t) * tokenlines_bufsize);
        } else {
            tokenlines_bufsize += TOKENLINES_INC;
            tokenlines           = (char ***)realloc(tokenlines, sizeof(char **) * tokenlines_bufsize);
            tokenlines_numtokens = (size_t*)realloc(tokenlines_numtokens, sizeof(size_t) * tokenlines_bufsize);
        }
    }
    if (numtokens == 0 || tokens == NULL) {
        tokenlines[numlines] = NULL;
        tokenlines_numtokens[numlines] = 0;
        numlines += 1;
        return 0;
    }
    tokenlines[numlines] = tokens;
    tokenlines_numtokens[numlines] = numtokens;
    numlines += 1;
    return numtokens;
}

size_t
getcolumnwidths(size_t numlines, char*** tokenlines) {
    size_t i;
    size_t j;
    size_t numtokens;
    size_t len;

    /* compute number of columns */
    numcolumns = 0;
    for (i = 0; i < numlines; i += 1) {
        if (numcolumns < tokenlines_numtokens[i]) {
            numcolumns = tokenlines_numtokens[i];
        }
    }

    /* initialize columnwidth array */
    columnwidth = (size_t*)malloc(sizeof(size_t) * numcolumns);
    for (j = 0; j < numcolumns; j += 1) {
        columnwidth[j] = 0;
    }

    /* compute column widths */
    for (i = 0; i < numlines; i += 1) {
        numtokens = tokenlines_numtokens[i];
        for (j = 0; j < numtokens; j += 1) {
            len = strlen(tokenlines[i][j]);
            if (columnwidth[j] < len) {
                columnwidth[j] = len;
            }
        }
    }

    return numlines;
}

void
printcolumns(size_t numlines, char*** tokenlines) {
    size_t i;
    size_t j;
    size_t numtokens;
    int width;
    for (i = 0; i < numlines; i += 1) {
        numtokens = tokenlines_numtokens[i];
        for (j = 0; j < numcolumns; j += 1) {
            width = (int)columnwidth[j];
            if (j < numtokens) {
                printf("%-*s", width, tokenlines[i][j]);
            } else {
                printf("%-*s", width, "");
            }
            if (j < (numcolumns - 1)) {
                fputs(" | ", stdout);
            } else {
                fputs("\n", stdout);
            }
        }
    }
}
