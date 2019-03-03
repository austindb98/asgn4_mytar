#ifndef UTIL
#define UTIL

#include <sys/stat.h>
#include "tarheader.h"


int addtoarchive(char *path, int fd);

int extract(char *path, char **targets, int numtargets);

void tarlist(char *filename, char **targets, int numtargets);
void tarlistVerbose(char *filename, char **targets, int numtargets);

char *makepath(header *fileheader);

char *dirtok(char *path);
void makedir(char *path, mode_t mode);

void printheader(header *fileheader);
int isheadernull(header *tarheader);
void skiptonextheader(header *fileheader, int fdTar);
int validateheader(header fileheader);

#endif
