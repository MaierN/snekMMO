#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

extern bool running;

void utils_err_check_no_exit(int status, char *msg);
void utils_err_check(int status, char *msg);
void utils_init_terminal();
void utils_setup_terminal();
void utils_restore_terminal();

#endif
