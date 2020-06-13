#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include <elf.h>

#include "sym.h"
#include "elph.h"
#include "debug.h"
#include "helper.h"

/*
 * elph_check() returns 0 if the file is a valid ELF
 */

int elph_check(char *buf, elph_data *data)
{
    Elf32_Ehdr *e = (Elf32_Ehdr*)buf;

    int code = 0;

    if(!e) code = -1;

    if(e->e_ident[EI_MAG0] != ELFMAG0) code = -2;
    if(e->e_ident[EI_MAG1] != ELFMAG1) code = -2;
    if(e->e_ident[EI_MAG2] != ELFMAG2) code = -2;
    if(e->e_ident[EI_MAG3] != ELFMAG3) code = -2;

    if(e->e_ident[EI_CLASS] != ELFCLASS32) code = -3;
    if(e->e_ident[EI_DATA] != ELFDATA2LSB) code = -4;
    if(e->e_ident[EI_VERSION] != EV_CURRENT) code = -5;

    if(e->e_type != ET_REL && e->e_type != ET_EXEC) code = -6;
    if(e->e_machine != EM_386) code = -7;

    switch(code) {
    case -1: elph_error("Read fail");
    case -2: elph_error("Invalid magic");
    case -3: elph_error("Not 32-bit");
    case -4: elph_error("Invalid endianness");
    case -5: elph_error("Not current version");
    case -6: 
		elph_error("Incorrect type");
		if(e->e_type == ET_DYN) data->elph_pie += 1;
	case -7: elph_error("Incorrect machine");
    }

    return code;
}

/*
 * stuff for elph_relocate() and the function itself.
 * relocates stuff, i guess.
 */

void elph_unknown()
{
	puts(ERROR"*** unknown function ***"NORM);
	_Exit(1);
}

struct elph_symbol elph_syms[] = {
	{"stdin", NULL},
	{"stdout", NULL},
	{"stderr", NULL},	
	{"__libc_start_main", &elph_sym_main},
	{NULL, NULL}
};

void elph_relocate(char *buf, int off, int shsize,
					Elf32_Sym *sym, const char *strings)
{
	Elf32_Rel *rel = (Elf32_Rel*)(buf + off);

	for(int i = 0; i < (int)(shsize / sizeof(*rel)); i++) {
		Elf32_Addr *addr = (Elf32_Addr*)(rel[i].r_offset);
		Elf32_Sym  *ssym = sym + ELF32_R_SYM(rel[i].r_info);
		const char *name = strings + ssym->st_name;
		Elf32_Addr *dsym = (Elf32_Addr*)dlsym(RTLD_DEFAULT, name);

		for(int i = 0; elph_syms[i].elph_name; i++) {
			if(!strcmp(elph_syms[i].elph_name, name)) {
				dsym = (Elf32_Addr*)elph_syms[i].elph_addr;
				break;
			}
		}

		elph_debug("REL:\taddr=0x%x name=\"%s\" dsym=0x%x", (int)addr, name, (int)dsym);

		if(!dsym) dsym = (Elf32_Addr*)&elph_unknown;

		switch(ELF32_R_TYPE(rel[i].r_info)) {
		case R_386_GLOB_DAT: *(Elf32_Addr*)addr = (int)dsym; break;
		case R_386_JMP_SLOT: *(Elf32_Addr*)addr = (int)dsym; break;
		case R_386_COPY: *(Elf32_Addr*)addr = (int)dsym; break;
		case R_386_32: *(Elf32_Addr*)addr += (int)dsym; break;
		}
	}
}

/*
 * elph_load_segments() reads out the program header table and
 * maps memory based on the addresses and offsets provided
 * 
 * TODO: implement PIE mapping
 */

void elph_load_segments(char *buf, elph_data *data)
{
	Elf32_Ehdr *e = (Elf32_Ehdr*)buf;
	Elf32_Phdr *p = (Elf32_Phdr*)(buf + e->e_phoff);

	data->elph_entry = (void*)e->e_entry;

	elph_debug("Entry: 0x%x", (int)data->elph_entry);

	for(int i = 0; i < e->e_phnum; i++) {
		int ts, pr;

		if(p[i].p_type == PT_LOAD) {
			if(p[i].p_flags & PF_X) pr |= PROT_EXEC;
			if(p[i].p_flags & PF_W) pr |= PROT_WRITE;
			if(p[i].p_flags & PF_R) pr |= PROT_READ;

			// from shinh's tel_ldr because idk how to round :/
			p[i].p_memsz += (p[i].p_vaddr & 0xfff);
			p[i].p_filesz += (p[i].p_vaddr & 0xfff);
			p[i].p_offset -= (p[i].p_vaddr & 0xfff);
			ts = (p[i].p_filesz + 0xfff) & ~0xfff;
			p[i].p_vaddr &= ~0xfff;
			p[i].p_memsz = (p[i].p_memsz + 0xfff) & ~0xfff;

			elph_debug("LOAD:\taddr=0x%x\tsize=0x%x", p[i].p_vaddr, ts);

			if(mmap((void*)p[i].p_vaddr, 
					ts, 
					pr, 
					MAP_PRIVATE|MAP_FIXED, 
					data->elph_fd, 
					p[i].p_offset) == MAP_FAILED) {
				elph_error("Error mapping 0x%x bytes to 0x%x",
							ts, p[i].p_vaddr);
				elph_errno();
			}

			if((pr & PROT_WRITE)) {
				for(; (int)p[i].p_filesz < ts; p[i].p_filesz++) {
					char *a = (char*)p[i].p_vaddr;
					a[p[i].p_filesz] = 0;
				}

				if(p[i].p_filesz != p[i].p_memsz) {
					if(mmap((void*)(p[i].p_vaddr + p[i].p_filesz),
							p[i].p_memsz - p[i].p_filesz,
							pr,
							MAP_PRIVATE|MAP_ANONYMOUS,
							-1,
							0) == MAP_FAILED) {
						elph_error("Error mapping 0x%x bytes to 0x%x",
							p[i].p_memsz - p[i].p_filesz,
							p[i].p_vaddr + p[i].p_filesz);
							elph_errno();
					}
				}
			}
		}
	}
}


/*
 * elph_load_sections() reads from the section header table and
 * looks for any sections under the SHT_DYNSYM type and the
 * SHT_REL type in order to do relocation magic and it also gets
 * the entry address to jump to once relocation is done
 */

void elph_load_sections(char *buf)
{
	Elf32_Ehdr *e = (Elf32_Ehdr*)buf;
	Elf32_Shdr *s = (Elf32_Shdr*)(buf + e->e_shoff);

	Elf32_Sym *syms = NULL;
	const char *strings = NULL;

	for(int i = 0; i < e->e_shnum; i++) {
		if(s[i].sh_type == SHT_DYNSYM) {
			syms = (Elf32_Sym*)(buf + s[i].sh_offset); 
			strings = buf + s[s[i].sh_link].sh_offset;
			break;
		}
	}

	for(int i = 0; i < e->e_shnum; i++) {
		if(s[i].sh_type == SHT_REL) {
			elph_relocate(buf, 
						(int)s[i].sh_offset,
						s[i].sh_size,
						syms,
						strings);
		}
	}
}

int elph_load(char *buf, int fd)
{
	elph_data *data = malloc(sizeof(elph_data));
	data->elph_fd = fd;
	data->elph_pie = 0;
	data->elph_entry = NULL;

	elph_syms[0].elph_addr = &stdin;
	elph_syms[1].elph_addr = &stdout;
	elph_syms[2].elph_addr = &stderr;

	if(!elph_check(buf, data)) {
		elph_load_segments(buf, data);
		elph_load_sections(buf);
		((void*(*)())(data->elph_entry))();
		free(data);
		return 0;
	} else {
		elph_error("Invalid ELF Header");
		free(data);
		return -1;
	}
}
