#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <sys/sysmacros.h>
#include "tarheader.h"

int addtoarchive(char *path, int fd) {
    header *file;

    file = buildheader(path);
    if(!file) {
        return -1;
    }

    write(fd,file,512);

    if(strcmp(file->typeflag,"5")) {
        
    }
}

header *buildheader(char *path) {
    struct stat filestat;
    char *typeflag;
    header *out = (header *)calloc(1,sizeof(header));
    if(!path || !out) {
        return NULL;
    }

    if(!lstat(path, &filestat)){
        return NULL;
    }

    if((filestat.st_mode & S_IFMT) == S_IFREG) {
        typeflag = "0";
    } else if((filestat.st_mode & S_IFMT) == S_IFLNK) {
        typeflag = "2";
    } else if((filestat.st_mode & S_IFMT) == S_IFDIR) {
        typeflag = "5";
    }

    char major[8];
    char minor[8];
    snprintf(major, 8, "%o", major(filestat.st_dev));
    snprintf(minor, 8, "%o", minor(filestat.st_dev));

    memcpy(out->name, path, strlen(path));
    memcpy(out->mode, &filestat.st_mode, 8);
    memcpy(out->uid, &filestat.st_uid, 8);
    memcpy(out->gid, &filestat.st_gid, 8);
    memcpy(out->size, &filestat.st_size, 12);
    memcpy(out->mtime, &filestat.st_mtime, 12);
    memset(out->chksum, ' ', 8);
    memcpy(out->typeflag, typeflag, 1);
    memcpy(out->linkname, &filestat.st_mode, 100);
    memcpy(out->magic, "ustar", 6);
    memcpy(out->version, "00", 2);
    memcpy(out->uname, getpwuid(filestat.st_uid)->pw_name, 32);
    memcpy(out->gname, getgrgid(filestat.st_gid)->gr_name, 32);
    memcpy(out->devmajor, major, 8);
    memcpy(out->devminor, minor, 8);
    memcpy(out->prefix, , 155);


}
