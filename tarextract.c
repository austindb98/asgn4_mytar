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
    FILE* tar = fopen(fileName, "r");

    if(tar == NULL) {
        perror("extract");
        return -1;
    }

    int len;
    fseek(tar, 0, SEEK_END);
    len = ftell(tar);
    fseek(tar, 0, SEEK_SET);
    header header;
    /*directory check*/
    while(ftell(tar) < len) {
        fread(&header, sizeof(header), 1, tar);

        if((header.name[0] != '\0') && (header.typeflag == '5')) {
            printf("Name of Directory: %s\n", header.name);
            makedir(header.name, atoi(header.mode));
            chown(header.name, atoi(header.uid), atoi(header.gid));
            setTime(header.name, atoi(header.mtime));
        }

        if(header.typeflag != '5') {
            int length = atoi(header.size) + getLen(atoi(header.size));
            fseek(tar, length, SEEK_CUR);
        }

    }
    /*non-directory checks*/
    fseek(tar, 0, SEEK_SET);
    while(ftell(tar) < len) {
        fread(&header, sizeof(header), 1, tar);
        if((header.name[0] != '2') && (header.typeflag != '5')){
            FILE* temp = open(header.name,O_RDONLY|O_WRONLY|O_CREAT|O_TRUNC);
            if(temp == NULL){
                perror("Could not open file");
                return -1;
            }
            int c;
            int length = atoi(header.size);
            /*not currently working ignore*/
            int cpt = 0;
            while(cpt < 1) {
                c = fgetc(tar);
                fputc(c, temp);
                cpt++;
            }
            fseek(tar, -1, SEEK_CUR);
            fclose(temp);


        }
        if(header.typeflag == '2') {
            int length = atoi(header.size) + getLen(atoi(header.size));
            fseek(tar, length, SEEK_CUR);
        }
    }
    fclose(tar);
    return 0;

}

/*temp main im using for testing*/
void main(int argc, char *argv[]) {
    int x = Extract(argv[1]);
    printf("%d\n", x);
}
