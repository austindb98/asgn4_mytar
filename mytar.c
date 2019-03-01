#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <limits.h>
#include "tarutil.h"

int main(int argc, char *argv[]) {
    int c,x,t,v,s;
    int tarfd;
    int i;

    if(argc <= 2) {
        fprintf(stderr,"Usage: mytar [ctxvS]f tarfile [ path [ ... ] ]");
        exit(EXIT_FAILURE);
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

    if(!(c||x||t)) {
        printf("Usage: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
        exit(EXIT_FAILURE);
    }

    if(c) {
        tarfd = open(argv[2],O_RDONLY|O_WRONLY|O_CREAT|O_TRUNC,0644);
        for(i = 3; i < argc; i++) {
            char *path = argv[i];
            addtoarchive(path,tarfd);
        }
        uint8_t block[512];
        memset(block,'\0',512);
        write(tarfd,block,512);
        write(tarfd,block,512);

        close(tarfd);
        exit(0);
    } else if(x) {
        char **targetfiles = NULL;
        /*assumes xf file targets*/
        if(argc > 3) {
            targetfiles = calloc(sizeof(char *),argc-3);
        }
        for(i = 3; i < argc; i++) {
            targetfiles[i-3] = argv[i];
        }
        extract(argv[2], targetfiles, argc-3);
        exit(0);
    } else if(t) {
        if(v){
            char **targetfiles = NULL;
            /*assumes xf file targets*/
            if(argc > 3) {
                targetfiles = calloc(sizeof(char *),argc-3);
            }
            for(i = 3; i < argc; i++) {
                targetfiles[i-3] = argv[i];
            }
            tarlistVerbose(argv[2], targetfiles, argc-3);
            exit(0);

        }
        char **targetfiles = NULL;
        /*assumes xf file targets*/
        if(argc > 3) {
            targetfiles = calloc(sizeof(char *),argc-3);
        }
        for(i = 3; i < argc; i++) {
            targetfiles[i-3] = argv[i];
        }
        tarlist(argv[2], targetfiles, argc-3);
        exit(0);

    }

}
