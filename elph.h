#ifndef ELPH_H
#define ELPH_H

#include <elf.h>

typedef struct
{
	// mmap stuff
	int elph_fd;

	// pie/pic
	int elph_pie;
	
	// jump here
	void *elph_entry;
} elph_data;

struct elph_symbol
{
	char *elph_name;
	void *elph_addr;	
};

struct elph_symbol elph_syms[9];

int elph_check(char*, elph_data*);

void *elph_get_symbol_dl(const char*, elph_data*);

void elph_relocate(char*, int, int, Elf32_Sym*, const char*);

void elph_load_segments(char*, elph_data*);

void elph_load_sections(char*);

int elph_load(char*, int);

#endif
