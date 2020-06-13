#ifndef DEBUG_H
#define DEBUG_H

#define NORM "\033[0m"
#define DEBUG "\033[0;36m"
#define ERROR "\033[0;31m"

#include <errno.h>
#include <string.h>

#ifndef NODEBUG
#define elph_debug(f, ...) \
	fprintf(stderr, DEBUG "DEBUG: "NORM f "\n", ## __VA_ARGS__)
#else
#define elph_debug(f, ...) \
    do { } while(0)
#endif

#define elph_error(f, ...) \
	fprintf(stderr, ERROR "ERROR: " NORM f "\n", ## __VA_ARGS__)

#define elph_errno() \
	elph_error("%s", strerror(errno))

#endif
