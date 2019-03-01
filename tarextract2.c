#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <utime.h>
#include <string.h>
#include "tarheader.h"
#include "tarutil.h"

#define DIRECTORY '5'
#define OCT 8

char *dirtok(char *path) {
    char *end;
    char *new_path;
    new_path = calloc(1,strlen(path));
    end = strrchr(path,'/');
    if((end != NULL)) {
        strncpy(new_path,path, end-path);
        printf("Changing to %s\n", new_path);
        chdir(new_path);
        return end;
    } else {
        printf("Remaining in %s\n", path);
        return path;
    }
}

void makedir(char *path, mode_t mode) {
    path[strlen(path)-1] = 0;
    char * newPath;
    newPath = dirtok(path);
    if(strcmp(newPath, path) == 0){
        mkdir(path, mode);
        chdir(path);
    }else{
        mkdir(newPath, mode);
    }
}

char *makepath(header *fileheader) {
    if(*(fileheader->prefix)) {
        char *out = calloc(1,256);
        strncpy(out,fileheader->prefix,155);
        strcat(out,"/");
        strncat(out,fileheader->name,100);
        return out;
    } else {
        return fileheader->name;
    }
}


/*
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
*/

int setTime(char *filename, time_t mtime) {
    struct utimbuf tmp;
    tmp.actime = 0;
    tmp.modtime = mtime;
    utime(filename, &tmp);
    return 0;
}

int extract(char *archivename) {
    int fdTar;
     fdTar = open(archivename, O_RDONLY);

    if(fdTar < 0) {
        perror("extract");
        return -1;
    }

    int len, size, mode, fdFile;
    uint8_t filesize;
    header fileheader;
    char *strbuff, *buf, *buf3;
    struct utimbuf modTime;
    //buf3 = malloc(512);
    //strbuff = malloc(512);
    char * path;
    /*directory check*/
    while((size = read(fdTar, &fileheader, 512)) != 0) {
        /*directory check*/
        if(fileheader.name[0] != '\0'
                && (*(fileheader.typeflag) == DIRECTORY)
                && (*(fileheader.typeflag) != 'L')) {

            path = makepath(&fileheader);
            printf("Name of Directory: %s\n", path);
            mode = strtol(fileheader.mode, &buf3, OCT);
            mkdir(path, mode);
            chown(path, strtol(fileheader.uid, &strbuff, OCT),
            strtol(fileheader.gid, &strbuff, OCT));
            setTime(path, strtol(fileheader.mtime, &strbuff, OCT));
            //chdir(fileheader.name);

        } else if(fileheader.typeflag[0] != DIRECTORY
                && *(fileheader.typeflag) != '\0'
                && *(fileheader.typeflag) != 'L') {

            printf("Extracting: %s\n", fileheader.name);
            path = makepath(&fileheader);
            mode = strtol(fileheader.mode, &buf3, OCT);
            fdFile = open(path,O_WRONLY|O_CREAT|O_TRUNC, mode);

            if(fdFile < 0) {
                fprintf(stderr, "cannot open file %s:", path);
                perror("");
                exit(EXIT_FAILURE);
            }

            int blocks,remainder;
            filesize = strtol(fileheader.size, &buf3, OCT);
            blocks = filesize/512;
            remainder = filesize%512;

            buf = calloc(513, sizeof(char));

            for(;blocks>0;--blocks) {
                if(read(fdTar, buf, 512)!=512) {
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                if(write(fdFile, buf, 512)!=512) {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
            }
            if(remainder) {
                if(read(fdTar, buf, 512)!=512) {
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                if(write(fdFile, buf, remainder)!=remainder) {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
            }

            free(buf);
            close(fdFile);
            modTime.modtime = strtol(fileheader.mtime, &buf3, OCT);
            modTime.actime = strtol(fileheader.mtime, &buf3, OCT);
            utime(path, &modTime);
        }
    }

    /*non-directory checks*/

    close(fdTar);
    return 0;
}
