#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <utime.h>
#include <string.h>
#include "tarheader.h"

#define DIRECTORY '5'
#define OCT 8

static void makedir(const char *dir, mode_t mode) {
    char tmp[256];
    char *x = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", dir);
    len = strlen(tmp);
    if(tmp[len - 1] == '/'){
        tmp[len - 1] = 0;
    }
    for(x = tmp + 1; *x; x++){
        if(*x == '/'){
            *x = 0;
            mkdir(tmp, mode);
            *x = '/';
        }
    }
    mkdir(tmp,mode);

}
int setTime(char *fileName, time_t mtime) {
    struct utimbuf tmp;

    tmp.actime = 0;
    tmp.modtime = mtime;
    utime(fileName, &tmp);

    return 0;
}

int Extract(char *fileName) {
    int fdTar;
     fdTar = open(fileName, O_RDONLY);

    if(fdTar < 0) {
        perror("extract");
        return -1;
    }
    int len, size, mode, fdFile;
    uint8_t fileSize;
    header headerFile;
    char *strbuff, *buf, *buf3;
    struct utimbuf modTime;
    //buf3 = malloc(512);
    //strbuff = malloc(512);

    /*directory check*/
    while((size = read(fdTar, &headerFile, sizeof(header))) != 0) {

        /*directory check*/
        if((headerFile.name[0] != '\0') && (*(headerFile.typeflag) == DIRECTORY)) {
            printf("Name of Directory: %s\n", headerFile.name);
            mode = strtol(headerFile.mode, &buf3, OCT);
            makedir(headerFile.name, mode);
            chown(headerFile.name, strtol(headerFile.uid, &strbuff, OCT), strtol(headerFile.gid, &strbuff, OCT));
            setTime(headerFile.name, strtol(headerFile.mtime, &strbuff, OCT));
        }

        fileSize = strtol(headerFile.size, &buf3, OCT);
        buf = calloc(fileSize, sizeof(char));
        mode = strtol(headerFile.mode, &buf3, OCT);
        size = read(fdTar, buf, fileSize);

        fdFile = open(headerFile.name,O_WRONLY|O_CREAT|O_TRUNC, 0644);
        if(fdFile < 0){
            perror("cannot open file");
            exit(EXIT_FAILURE);
        }
        size = write(fdFile, buf, fileSize);

        if(size != fileSize){
            perror("error in writing file");
        }
        
        free(buf);
        close(fdFile);
        modTime.modtime = strtol(headerFile.mtime, &buf3, OCT);
        modTime.actime = strtol(headerFile.mtime, &buf3, OCT);
        utime(headerFile.name, &modTime);


    }
    /*non-directory checks*/
    
  
    close(fdTar);
    return 0;

}

/*temp main im using for testing*/
void main(int argc, char *argv[]) {
    int x = Extract(argv[1]);
    printf("%d\n", x);
}
