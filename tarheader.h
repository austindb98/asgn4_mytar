#ifndef HEADER
#define HEADER
#include <stdint.h>

typedef struct{
	uint8_t name[100];
	uint8_t mode[8];
	uint8_t uid[8];
	uint8_t gid[8];
	uint8_t size[12];
	uint8_t mtime[12];
	uint8_t chksum[8];
	uint8_t typeflag[1];
	uint8_t linkname[100];
	uint8_t magic[6];
	uint8_t version[2];
	uint8_t uname[32];
	uint8_t gname[32];
	uint8_t devmajor[8];
	uint8_t devminor[8];
	uint8_t prefix[155];
	uint8_t null[12];
} header;
#endif
