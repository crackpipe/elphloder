#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <elf.h>

#include "sym.h"
#include "elph.h"
#include "debug.h"
#include "helper.h"

void help(char *b)
{
	printf("elphloder by eight\n"
		"Compiled at: " __DATE__ " " __TIME__ "\n"
		"Usage: %s <elf> <args>\n", b);
	_Exit(0);
}

int main(int argc, char **argv)
{
	if(argc < 2) help(argv[0]);
	else {
		int fd;
		size_t filesz;

		elph_argc = argc - 1;
		elph_argv = argv + 1;

		if((fd = open(argv[1], O_RDONLY)) > 0) {
			filesz = lseek(fd, 0, SEEK_END);

			char buffer[filesz + 1];

			h_seek_fd(fd, 0, h_seek_err);
			h_read_fd(fd, &buffer, filesz, h_read_err);

			elph_syms[0].elph_addr = &stdin;
			elph_syms[1].elph_addr = &stdout;
			elph_syms[2].elph_addr = &stderr;

			if(!elph_load(buffer, fd)) {
				elph_debug("OK");
			}

			close(fd);
		} else {
			close(fd);
			elph_error("Open fail");
		}
	}

	return 0;
}
