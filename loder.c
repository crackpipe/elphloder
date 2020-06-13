#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

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
	exit(0);
}

void signal_handler(int sig)
{
	elph_error("Caught signal. %s", sys_siglist[sig]);
	exit(1);
}

int main(int argc, char **argv)
{
	for(int i = 1; i < NSIG; i++) {
		signal(i, signal_handler);
	}

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
