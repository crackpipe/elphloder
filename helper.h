#ifndef HELPER_H
#define HELPER_H

#include <sys/types.h>

#define h_off(fd) lseek(fd, 0, SEEK_CUR)

void h_seek_err(int);

void h_read_err(int);

void h_seek_fd(int, off_t, void (*)(int));

void h_read_fd(int, void*, size_t, void (*)(int));

#endif
