#ifndef TSVTABLE_H
#define TSVTABLE_H

#include <unistd.h>
#include <stdio.h>

#define LINESIZE 65536
#define TOKENLINES_INC 4096
#define LINESINC 4096
#define MAXCOLUMNS 256

extern char linebuf[LINESIZE];
extern char **lines;
extern size_t linebufsize;
extern size_t numlines;
extern size_t numcolumns;
extern size_t columnwidth[MAXCOLUMNS];
extern size_t bytes;
extern size_t alloced;
extern char tokenbuf[LINESIZE];
extern char* tokens[MAXCOLUMNS];
extern size_t numtokens;

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

#endif
