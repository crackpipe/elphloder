#ifndef SYM_H
#define SYM_H

int elph_argc;
char **elph_argv;

int elph_sym_main(int (*)(int, char**, char**), int, char**);

#endif
