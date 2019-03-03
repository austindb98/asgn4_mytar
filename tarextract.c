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

int setTime(char *filename, time_t mtime) {
    struct utimbuf tmp;
    tmp.actime = 0;
    tmp.modtime = mtime;
    utime(filename, &tmp);
    return 0;
}

int extract(char *archivename, char **targets, int numtargets) {
    int fdTar;
     fdTar = open(archivename, O_RDONLY);

    if(fdTar < 0) {
        perror("extract");
        return -1;
    }

    int mode, fdFile;
    header fileheader;
    char *strbuff, *buf, *buf3;
    struct utimbuf modTime;
    char * path;
    int i;
    int pastheadernull = 0;


    while(read(fdTar, &fileheader, 512) == 512) {
        /*null block check*/
        if(isheadernull(&fileheader)) {
            //printf("--- Current header null ---\n");
            if(pastheadernull) {
            //    printf("-     BREAKING     -\n");
                break;
            }
        }
        pastheadernull = isheadernull(&fileheader);

        /*if targets check*/
        int targetflag = targets?0:1;
        path = makepath(&fileheader);
        for(i =0; i < numtargets; i++) {
            char *lastslashTarget = strrchr(targets[i], '/');
            char *lastslashPath = strrchr(targets[i], '/');
            if(lastslashTarget == NULL){
                if(lastslashPath == NULL){
                    if(!strncmp(path, targets[i], strlen(targets[i]))){
                        targetflag = 1;
                    }
                }

            }else if(lastslashPath == '/' || strlen(path) <= 
                (strlen(targets[i]) - (strlen(lastslashTarget) - 1))){
                if(!strncmp(path, targets[i], strlen(path))){
                    targetflag = 1;
                }
            }else{
                if(!strncmp(path, targets[i], strlen(targets[i]))){
                    targetflag = 1;
                }

        }
        }

        /*If we need this file*/
        if(targetflag) {
            /* If directory */
            if(fileheader.name[0] != '\0'
                    && (*(fileheader.typeflag) == DIRECTORY)
                    && (*(fileheader.typeflag) != '2')) {

                //printf("Name of Directory: %s\n", path);
                mode = strtol(fileheader.mode, &buf3, OCT);
                mkdir(path, mode);
                chown(path, strtol(fileheader.uid, &strbuff, OCT),
                strtol(fileheader.gid, &strbuff, OCT));
                setTime(path, strtol(fileheader.mtime, &strbuff, OCT));

            /*If regular file*/
            } else if(fileheader.typeflag[0] != DIRECTORY
                    && *(fileheader.typeflag) != '\0' 
                    && *(fileheader.typeflag) != '2'){

                path = makepath(&fileheader);
                mode = strtol(fileheader.mode, &buf3, OCT);
                fdFile = open(path,O_WRONLY|O_CREAT|O_TRUNC, mode);

                if(fdFile < 0) {
                    fprintf(stderr, "Cannot open file %s:", path);
                    perror("");
                    exit(EXIT_FAILURE);
                }

                int bytes;
                bytes = strtol(fileheader.size, &buf3, OCT);
                buf = calloc(513, sizeof(char));

                /*Copy file to output*/
                for(;bytes>=512;bytes-=512) {
                    if(read(fdTar, buf, 512)!=512) {
                        perror("read");
                        exit(EXIT_FAILURE);
                    }
                    if(write(fdFile, buf, 512)!=512) {
                        perror("write");
                        exit(EXIT_FAILURE);
                    }
                }
                if(bytes) {
                    if(read(fdTar, buf, 512)!=512) {
                        perror("read");
                        exit(EXIT_FAILURE);
                    }
                    if(write(fdFile, buf, bytes)!=bytes) {
                        perror("write");
                        exit(EXIT_FAILURE);
                    }
                }

                free(buf);
                close(fdFile);
                modTime.modtime = strtol(fileheader.mtime, &buf3, OCT);
                modTime.actime = strtol(fileheader.mtime, &buf3, OCT);
                utime(path, &modTime);

            /*If Link*/
            } else if(fileheader.typeflag[0] != DIRECTORY
                    && (*(fileheader.typeflag) == '2'
                    || *(fileheader.typeflag) == 'L')) {
                char *temp = calloc(1,101);
                strncpy(temp,fileheader.linkname,100);
                printf("Link: %s\n",temp);
                symlink(temp,path);
                free(temp);
            }
        /*These aren't the files we're looking for*/
        } else {
            /*Skip correct number of blocks*/
            int bytes = strtol(fileheader.size, &buf3, OCT);
            buf = calloc(513, sizeof(char));

            skiptonextheader(&fileheader,fdTar);
        }
    }

    close(fdTar);
    return 0;

}
