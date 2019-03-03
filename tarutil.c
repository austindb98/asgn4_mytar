#include "tarutil.h"
#include "tarheader.h"
#include <string.h>
#include <stdlib.h>


char *makepath(header *fileheader) {
    char *out = calloc(1,256);
    if(*(fileheader->prefix)) {

        strncpy(out,fileheader->prefix,155);
        strcat(out,"/");
        strncat(out,fileheader->name,100);
        return out;
    } else {
        strncpy(out,fileheader->name,100);
        return out;
    }
}

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

int isheadernull(header *tarheader) {
    int i;
    for(i = 0; i < 512; i++) {
        if(((uint8_t *)tarheader)[i]) {
            return 0;
        }
    }
    return 1;
}

void skiptonextheader(header *fileheader, int fdTar) {
    int bytes = strtol(fileheader->size, NULL, 8);
    char *buf = calloc(513, sizeof(char));

    for(;bytes>=512;bytes-=512) {
        if(read(fdTar, buf, 512)!=512) {
            perror("read");
            exit(EXIT_FAILURE);
        }
    }

    if(bytes){
        if(read(fdTar, buf, 512)!=512) {
            perror("read");
            exit(EXIT_FAILURE);
        }
    }
}

int validateheader(header *fileheader) {
    header *new = fileheader;
    int chksum = strtol(fileheader->chksum, NULL, 8);
    int newchksum = 0;
    int i;
    uint8_t *byteptr = (void *)fileheader;

    memset(new->chksum, ' ', 8);

    for(i = 0; i < 512; i++) {
        printf("Reading byte no. %d\n",byteptr[i]);
        newchksum += byteptr[i];
    }
    printf("Actual sum: %d\n", chksum);
    printf("Calculated sum: %d\n", newchksum);
    return newchksum == chksum;
}
