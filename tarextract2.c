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

void printheader(header *fileheader) {
    uint8_t buf[256];
    memset(buf,'\0',256);
    memcpy(buf,fileheader->name,100);
    printf("name: %s\n",buf);
    memset(buf,'\0',256);
    memcpy(buf,fileheader->mode,8);
    printf("mode: %s\n",buf);
    memset(buf,'\0',256);
    memcpy(buf,fileheader->uid,8);
    printf("uid: %s\n",buf);
    memset(buf,'\0',256);
    memcpy(buf,fileheader->gid,100);
    printf("gid: %s\n",buf);
    memset(buf,'\0',256);
    memcpy(buf,fileheader->size,12);
    printf("size: %s\n",buf);
    memset(buf,'\0',256);
    memcpy(buf,fileheader->mtime,12);
    printf("mtime: %s\n",buf);
    memset(buf,'\0',256);
    memcpy(buf,fileheader->chksum,8);
    printf("chksum: %s\n",buf);
    memset(buf,'\0',256);
    memcpy(buf,fileheader->typeflag,1);
    printf("typeflag: %s\n",buf);
    memset(buf,'\0',256);
    memcpy(buf,fileheader->linkname,100);
    printf("linkname: %s\n",buf);
    memset(buf,'\0',256);
    memcpy(buf,fileheader->magic,6);
    printf("magic: %s\n",buf);
    memset(buf,'\0',256);
    memcpy(buf,fileheader->version,2);
    printf("version: %s\n",buf);
    memset(buf,'\0',256);
    memcpy(buf,fileheader->uname,32);
    printf("uname: %s\n",buf);
    memset(buf,'\0',256);
    memcpy(buf,fileheader->gname,32);
    printf("gname: %s\n",buf);
    memset(buf,'\0',256);
    memcpy(buf,fileheader->devmajor,8);
    printf("devmajor: %s\n",buf);
    memset(buf,'\0',256);
    memcpy(buf,fileheader->devminor,8);
    printf("devminor: %s\n",buf);
    memset(buf,'\0',256);
    memcpy(buf,fileheader->prefix,155);
    printf("prefix: %s\n",buf);
    memset(buf,'\0',256);
    memcpy(buf,fileheader->null,12);
    printf("null?: %s\n\n",buf);
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

int isheadernull(header *tarheader) {
    int i;
    for(i = 0; i < 512; i++) {
        if(((uint8_t *)tarheader)[i]) {
            return 0;
        }
    }
    return 1;
}

int validateheader(header *tarheader) {
    header cpy;
    int i,sum=0;
    char *strsum = calloc(1,8);
    memcpy(&cpy,tarheader,512);
    memset(cpy.chksum,' ',8);
    for(i = 0; i < 512; i++) {
        sum+=((uint8_t *)tarheader)[i];
    }
    snprintf(strsum, 8, "%0*o", 7, sum);
    return strncmp(strsum,tarheader->chksum,8);
}

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

    int mode, fdFile;
    header fileheader;
    char *strbuff, *buf, *buf3;
    struct utimbuf modTime;
    char * path;
    int pastheadernull = 0;

    /*directory check*/
    while(read(fdTar, &fileheader, 512) == 512) {
        if(isheadernull(&fileheader)) {
            printf("--- Current header null ---\n");
            if(pastheadernull) {
                printf("-     BREAKING     -\n");
                break;
            }
        }
        pastheadernull = isheadernull(&fileheader);
        /*if(validateheader(&fileheader)) {
            fprintf(stderr, "---BAD HEADER---\n");
        }*/
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

            int bytes;
            bytes = strtol(fileheader.size, &buf3, OCT);

            buf = calloc(513, sizeof(char));

            //printf("File size: %dB\n", bytes);
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
                //printf("bytes left (last pull): %dB\n", bytes);
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
        } else if(fileheader.typeflag[0] != DIRECTORY
                && (*(fileheader.typeflag) == '2'
                || *(fileheader.typeflag) == 'L')) {
            char *temp = calloc(1,101);
            strncpy(temp,fileheader.linkname,100);
            printf("Link: %s\n",temp);
            symlink(temp,path);
            free(temp);
        }
    }

    close(fdTar);
    return 0;
}
