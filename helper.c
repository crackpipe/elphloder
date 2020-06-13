#include <stdio.h>
#include <unistd.h>

#include "debug.h"
#include "helper.h"

void h_seek_err(int h_fd)
{
	close(h_fd);

	elph_error("Seek fail");
	elph_errno();

	_Exit(1);
}

void h_read_err(int h_fd)
{
	close(h_fd);

	elph_error("Read fail");
	elph_errno();

	_Exit(1);
}

void h_seek_fd(int h_fd, off_t h_off, void (*h_callback)(int))
{
	if(lseek(h_fd, h_off, SEEK_SET) != h_off)
		h_callback(h_fd);
}

void h_read_fd(int h_fd, void *h_buf, size_t h_sz, void (*h_callback)(int))
{
	if(read(h_fd, h_buf, h_sz) != (ssize_t)h_sz)
		h_callback(h_fd);
}
