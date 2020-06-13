#include <errno.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include "sym.h"

int elph_sym_main(int (*elph_main)(int, char**, char**),
					int argc, char **argv)
{
	if(argc) {
		argc = elph_argc;
		argv = elph_argv;
	}

	exit(elph_main(argc, argv, NULL));
}
