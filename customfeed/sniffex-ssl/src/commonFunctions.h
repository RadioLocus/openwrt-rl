#include<stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

char* allocateString(char *);
char* combineString(char *, char*);
char *str_replace ( const char *, const char *, const char *);
//void str_append(char *, char *);
int add(int,int);
char * strautocat(char **, const char *);
int get_normalized_time(int timestamp, int timenormalizer);
double string_to_double(char *);
