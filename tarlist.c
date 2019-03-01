#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <utime.h>
#include <string.h>
#include "tarheader.h"
#include <time.h>
#define S_IFMT 0160000 /* type of file: */
#define S_IFDIR 0040000 /* directory */


#define DIRECTORY '5'
#define OCT 8

typedef struct {
    int flag;
    int index;
    int val;
}permChar;



char *makepath(header *fileheader) {
    if(*(fileheader->prefix)) {
        char *out = calloc(1,256);
        strncpy(out,fileheader->prefix,155);
        strncat(out,"/",1);
        strncat(out,fileheader->name,100);
        return out;
    } else {
        return fileheader->name;
    }
}




void tarlist(char *filename, char **targets, int numtargets){
    int fdTar;
     fdTar = open(filename, O_RDONLY);

    if(fdTar < 0) {
        perror("listing");
        return -1;
    }
    int len, size, mode, fdFile, i;
    uint8_t fileSize;
    header headerFile;
    char *strbuff, *buf, *buf3;
    struct utimbuf modTime;
    char * path;
    while((size = read(fdTar, &headerFile, 512)) != 0) {
        /*if targets check*/
        int targetflag = targets?0:1;
        path = makepath(&headerFile);
        for(i =0; i < numtargets; i++) {
            if(!strcmp(path,targets[i])) {
                targetflag = 1;
            }
        }
        if(targetflag == 1){
            if((headerFile.name[0] != '\0')  
                    && (*(headerFile.typeflag) != 'L')) {

                if((headerFile.typeflag[0] == '0') 
                        || (headerFile.typeflag[0] == DIRECTORY)){
                path = makepath(&headerFile.name);
                printf("%s\n", path);
              }   
            }
        
        }
        }

    close(fdTar);

}

void tarlistVerbose(char *filename, char **targets, int numtargets){
    int fdTar;
     fdTar = open(filename, O_RDONLY);

   static permChar PERMCHARS[] = {
   {S_IRUSR, 1, 'r'}, {S_IWUSR, 2, 'w'}, {S_IXUSR, 3, 'x'},
   {S_IRGRP, 4, 'r'}, {S_IWGRP, 5, 'w'}, {S_IXGRP, 6, 'x'},
   {S_IROTH, 7, 'r'}, {S_IWOTH, 8, 'w'}, {S_IXOTH, 9, 'x'},
   {S_ISUID, 3, 's'}, {S_ISGID, 6, 's'}, {S_IFDIR, 0, 'd'},
   {0, 0, 0}
};


    if(fdTar < 0) {
        perror("listing");
        return -1;
    }
    int len, size, mode, fdFile, i;
    uint8_t fileSize;
    header headerFile;
    permChar *pChar;
    char *timebuf[80];
    char  *buf;
    char permStr[11], *timeStr;
    time_t mtime;
    char * path;
    struct tm *info;
    while((size = read(fdTar, &headerFile, 512)) != 0) {

        /*if targets check*/
        int targetflag = targets?0:1;
        path = makepath(&headerFile);
        for(i =0; i < numtargets; i++) {
            if(!strcmp(path,targets[i])) {
                targetflag = 1;
            }
        }
        if(targetflag == 1){

            if((headerFile.name[0] != '\0')  
                    && (*(headerFile.typeflag) != 'L')) {

                if((headerFile.typeflag[0] == '0') 
                        || (headerFile.typeflag[0] == DIRECTORY)){
                
                mode = strtol(headerFile.mode, &buf, OCT);
                for(i = 0; i < 10; i++){
                    permStr[i] = '-';    
                }
                permStr[i] = 0;

                for(pChar = PERMCHARS; pChar->flag; pChar++){
                    if(pChar -> flag == 16384){
                        if(*headerFile.typeflag == DIRECTORY){
                            permStr[pChar->index] = pChar -> val;
                        }
                    }else if(mode & pChar -> flag){
                        permStr[pChar->index] = pChar -> val;
                    }
                }
                mtime = strtol(headerFile.mtime, &buf, OCT);
                time(&mtime);
                info = localtime(&mtime);
                strftime(timebuf, 80, "%Y-%m-%d %H:%M", info);
                path = makepath(&headerFile.name);
                size = strtol(headerFile.size, &buf, OCT);
                printf("%s %s %s %9u %s %s\n", permStr, 
                        headerFile.uname, headerFile.gname,
                        size, timebuf, path);
              }   
            }
        

        }
    }

    close(fdTar);

}

