#pragma once

#include <sys/types.h>
#include <time.h>
#include <stdio.h>

#ifndef _MAX_PATH
#define _MAX_PATH 260
#endif

// File attribute constants used by _finddata_t::attrib
#define _A_NORMAL 0x00
#define _A_RDONLY 0x01
#define _A_HIDDEN 0x02
#define _A_SYSTEM 0x04
#define _A_SUBDIR 0x10
#define _A_ARCH   0x20

struct _finddata_t {
    unsigned    attrib;
    time_t      time_create;
    time_t      time_access;
    time_t      time_write;
    off_t       size;
    char        name[_MAX_PATH];
};

#ifdef __cplusplus
extern "C" {
#endif

long _findfirst(const char *filespec, struct _finddata_t *fileinfo);
int _findnext(long handle, struct _finddata_t *fileinfo);
int _findclose(long handle);

// fopen with Windows-style case-insensitive path resolution on read modes.
FILE* fopen_ci(const char* path, const char* mode);

#ifdef __cplusplus
}
#endif