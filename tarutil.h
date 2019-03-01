int addtoarchive(char *path, int fd);
int extract(char *path, char **targets, int numtargets);
void tarlist(char *filename, char **targets, int numtargets);
void tarlistVerbose(char *filename, char **targets, int numtargets);
