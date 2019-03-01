#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include "tarcreate.h"

int main(int argc, char *argv[]) {
    int c,x,t,v,s;
    int tarfd;
    int i;

    if(argc <= 1) {
        printf("Usage: mytar [ctxvS]f tarfile [ path [ ... ] ]");
        exit(-1);
    }

    c=0,x=0,t=0,v=0,s=0;
    if(strchr(argv[1],'c')) {
        c=1;
    }
    if(strchr(argv[1],'x')) {
        x=1;
    }
    if(strchr(argv[1],'t')) {
        t=1;
    }
    if(strchr(argv[1],'v')) {
        v=1;
    }
    if(strchr(argv[1],'S')) {
        s=1;
    }

    if(c) {
        tarfd = open(argv[2],O_RDONLY|O_WRONLY|O_CREAT|O_TRUNC,0644);
        for(i = 3; i < argc; i++) {
            addtoarchive(argv[i],tarfd);
        }
        uint8_t block[512];
        memset(block,'\0',512);
        write(tarfd,block,512);
        write(tarfd,block,512);

        close(tarfd);
    }

}