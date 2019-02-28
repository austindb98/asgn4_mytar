#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/sysmacros.h>
#include "tarcreate.h"
#include "tarheader.h"

/*Convert and format octal ascii strings*/
void octaltoasciiset(void *target, unsigned int in, int len) {
    uint8_t *octal_buffer;
    octal_buffer = (uint8_t *)malloc(len);
    memset(octal_buffer, ' ', len);
    snprintf(octal_buffer, len, "%0*o", len-1, in);
    memcpy(target, octal_buffer, len);
    free(octal_buffer);
}

header *buildheader(char *path) {
    struct stat filestat;
    char *typeflag;
    int i;
    int chksum;
    char *name_ptr = NULL;
    chksum = 0;

    header *out = (header *)calloc(1,sizeof(header));
    /*Check conditions*/
    if(!path || !out || strlen(path) > 256) {
        if(!path) {
            printf("Path failure\n");
        } else if (!out) {
            perror("malloc");
        } else {
            printf("Path too long");
        }
        return NULL;
    }

    if(lstat(path, &filestat)){
        perror("lstat");
        return NULL;
    }

    if((filestat.st_mode & S_IFMT) == S_IFREG) {
        typeflag = "0";
    } else if((filestat.st_mode & S_IFMT) == S_IFLNK) {
        typeflag = "2";
        readlink(path, out->linkname, 100);
    } else if((filestat.st_mode & S_IFMT) == S_IFDIR) {
        typeflag = "5";
    }

    name_ptr = path;
    if(strlen(path) <= 100) {
        memcpy(out->name, path, strlen(path));
    } else {
        while(strlen(name_ptr) > 100) {
            name_ptr = strrchr(name_ptr,'/')+1;
        }
        memcpy(out->name, path, strlen(path));
    }
    strncpy(out->prefix, path, name_ptr-path);

    octaltoasciiset(out->mode, filestat.st_mode, 8);
    octaltoasciiset(out->uid, filestat.st_uid, 8);
    octaltoasciiset(out->gid, filestat.st_gid, 8);
    octaltoasciiset(out->size, filestat.st_size, 12);
    octaltoasciiset(out->mtime, filestat.st_mtime, 12);
    memset(out->chksum, ' ', 8);
    memcpy(out->typeflag, typeflag, 1);
    memcpy(out->magic, "ustar", 6);
    memcpy(out->version, "00", 2);
    memcpy(out->uname, getpwuid(filestat.st_uid)->pw_name, 32);
    memcpy(out->gname, getgrgid(filestat.st_gid)->gr_name, 32);
    octaltoasciiset(out->devmajor, major(filestat.st_dev), 8);
    octaltoasciiset(out->devminor, minor(filestat.st_dev), 8);

    for(i = 0; i<512; i++) {
        chksum += ((uint8_t *)out)[i];
    }
    octaltoasciiset(out->chksum,chksum,8);

    return out;
}

int addtoarchive(char *path, int fd) {
    header *file;
    int cur_file;
    DIR *current_dir;
    uint8_t buffer[512];
    struct dirent *current_dirent;
    struct stat self;
    struct stat parent;
    char *pathparent = calloc(1,strlen(path) + 4);

    /*printf("Building header for: %s\n", path);*/

    file = buildheader(path);
    if(!file) {
        return -1;
    }

    write(fd,file,512);
    /*printf("Wrote header to file\n");*/

    if(!strncmp(file->typeflag, "5", 1)) {
        /*printf("File is directory\n");*/
        current_dir = opendir(path);
        if(!current_dir) {
            return -2;
        }

        while((current_dirent = readdir(current_dir))) {
            lstat(path, &self);
            strcpy(pathparent,path);
            strcat(pathparent,"/..");
            lstat(pathparent, &parent);

            if(current_dirent->d_ino != self.st_ino
                && current_dirent->d_ino != parent.st_ino) {
                char *new_path;
                new_path = calloc(1,
                        strlen(path) + strlen(current_dirent->d_name) + 2);
                        /* Concat with '/' + '\0' */
                strncpy(new_path, path, strlen(path));
                strncat(new_path,"/",1);
                strncat(new_path, current_dirent->d_name, strlen(current_dirent->d_name));
                /*printf("Adding to archive: %s", new_path);*/
                addtoarchive(new_path, fd);
                free(new_path);
            }
        }

        closedir(current_dir);
    } else {
        cur_file = open(path, O_RDONLY);
        memset(buffer, '\0', 512);
        while(read(cur_file, buffer, 512)>0) {
            /*printf("Copying file\n");*/
            write(fd, buffer, 512);
            memset(buffer, '\0', 512);
        }
    }
    return 0;
}
