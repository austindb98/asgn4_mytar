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
#define OCT '8'

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

int getLen(int x) {
    if( x == 512) {
        return 0;
    } else {
        return(512 -(x % 512));
    }
}

int Extract(char *fileName) {
    int fdTar;
     fdTar = open(fileName, O_RDONLY);

    if(fdTar < 0) {
        perror("extract");
        return -1;
    }
    int len, size, mode, fdFile;
    ssize_t fileSize;
    Header header;
    char *strbuff, *buf, *buf3;
    struct utimbuf modTime;

    /*directory check*/
    while((size = read(fdTar, &header, sizeof(Header))) != 0) {

        /*directory check*/
        if((header.name[0] != '\0') && (*(header.typeflag) == DIRECTORY)) {
            printf("Name of Directory: %s\n", header.name);
            mode = strtol(header.mode, &buf3, OCT);
            mkdir(header.name, mode);
            chown(header.name, strtol(header.uid, &strbuff, OCT), strtol(header.gid, &strbuff, OCT));
            setTime(header.name, strtol(header.mtime, &strbuff, OCT));
        }

        fileSize = strtol(header.size, &buf3, OCT);
        buf = calloc(fileSize, sizeof(char));
        mode = strtol(header.mode, &buf3, OCT);
        size = read(fdTar, buf, fileSize);

        fdFile = open(header.name,O_CREAT | O_TRUNC | O_WRONLY, mode);
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
        modTime.modtime = strtol(header.mtime, &buf3, OCT);
        modTime.actime = strtol(header.mtime, &buf3, OCT);
        utime(header.name, &modTime);


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
