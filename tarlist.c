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
#include <time.h>

#define S_IFMT 0160000 /* type of file: */
#define S_IFDIR 0040000 /* directory */

#define DIRECTORY '5'
#define OCT 8

/*struct for the permission characters*/
typedef struct {
    int flag;
    int index;
    int val;
}permChar;

/*simple version of list that only 
lists the names of the archived files/directories/symlinks*/
void tarlist(char *filename, char **targets, int numtargets){
    int fdTar;
     fdTar = open(filename, O_RDONLY);
     /*can we open?*/
    if(fdTar < 0) {
        perror("listing");
        exit(-1);
    }
    int len, size, mode, fdFile, i;
    uint8_t fileSize;
    header fileheader;
    char *strbuff, *buf, *buf3;
    struct utimbuf modTime;
    char *path;
    /*read in each block*/
    while((size = read(fdTar, &fileheader, 512)) != 0) {
        int targetflag = targets?0:1;
        path = makepath(&fileheader);
        for(i =0; i < numtargets; i++) {
            if(!strncmp(path, targets[i], strlen(targets[i]))) {
                targetflag = 1;
            }
        }
        /*is it the correct file type?*/
        if(targetflag == 1){
            if(fileheader.name[0] != '\0'
                    && (fileheader.typeflag[0] == '0'
                    ||  fileheader.typeflag[0] == '\0'
                    ||  fileheader.typeflag[0] == '2'
                    ||  fileheader.typeflag[0] == DIRECTORY)) {

                path = makepath(&fileheader.name);
                printf("%s\n", path);
                skiptonextheader(&fileheader,fdTar);
            }
        } else {
            skiptonextheader(&fileheader,fdTar);
        }

    }
    close(fdTar);
}
/*Verbose version of list that 
prints out extra information about 
each file/directory/symlink*/
void tarlistVerbose(char *filename, char **targets, int numtargets){
    int fdTar;
     fdTar = open(filename, O_RDONLY);
     /*array holding the possible flags 
     with their respective indexes and values*/
    static permChar PERMCHARS[] = {
    {S_IRUSR, 1, 'r'}, {S_IWUSR, 2, 'w'}, {S_IXUSR, 3, 'x'},
    {S_IRGRP, 4, 'r'}, {S_IWGRP, 5, 'w'}, {S_IXGRP, 6, 'x'},
    {S_IROTH, 7, 'r'}, {S_IWOTH, 8, 'w'}, {S_IXOTH, 9, 'x'},
    {S_ISUID, 3, 's'}, {S_ISGID, 6, 's'}, {S_IFDIR, 0, 'd'},
    {S_IFLNK, 0, 'l'}, {0, 0, 0}
    };

    /*can we open?*/
    if(fdTar < 0) {
        perror("listing");
        exit(-1);
    }
    int len, size, mode, fdFile, i;
    uint8_t fileSize;
    header fileheader;
    permChar *pChar;
    char timebuf[80];
    char  *buf;
    char permStr[11], *timeStr;
    time_t mtime;
    char * path;
    struct tm *info;
    /*read in each block*/
    while((size = read(fdTar, &fileheader, 512)) != 0) {

        /*if targets check*/
        int targetflag = targets?0:1;
        path = makepath(&fileheader);
        for(i =0; i < numtargets; i++) {
            if(!strncmp(path, targets[i], strlen(targets[i]))) {
                targetflag = 1;
            }
        }
        /*is it the correct file type?*/
        if(targetflag == 1){
            if(fileheader.name[0] != '\0' && 
                       (fileheader.typeflag[0] == '0'
                    ||  fileheader.typeflag[0] == '\0'
                    ||  fileheader.typeflag[0] == '2'
                    ||  fileheader.typeflag[0] == DIRECTORY)) {

                mode = strtol(fileheader.mode, &buf, OCT);
                for(i = 0; i < 10; i++) {
                    permStr[i] = '-';
                }
                permStr[i] = 0;
                /*is it a directory?*/
                for(pChar = PERMCHARS; pChar->flag; pChar++){
                    if(pChar -> flag == 16384){
                        if(*fileheader.typeflag == DIRECTORY){
                            permStr[pChar->index] = pChar -> val;
                        }
                    /*is it a symlink?*/
                    }else if(pChar -> flag == S_IFLNK){
                        if(*fileheader.typeflag == '2'){
                            permStr[pChar->index] = pChar -> val;
                    }
                    /*normal file*/
                    }else if(mode & pChar -> flag){
                        permStr[pChar->index] = pChar -> val;
                    }
                }

                mtime = strtol(fileheader.mtime, &buf, OCT);
                info = localtime(&mtime);
                strftime(timebuf, 80, "%Y-%m-%d %H:%M", info);
                path = makepath(&fileheader.name);
                size = strtol(fileheader.size, &buf, OCT);
                /*print out the final info*/
                printf("%s %s/%s %9u %s %s\n", permStr,
                        fileheader.uname, fileheader.gname,
                        size, timebuf, path);

                skiptonextheader(&fileheader,fdTar);
            }
        } else {
            skiptonextheader(&fileheader,fdTar);
        }

    }

    close(fdTar);

}
