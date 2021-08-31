#ifndef UTILS_H
#define UTILS_H

void utils_err_check(int status, char *msg);
void utils_init_terminal();
void utils_setup_terminal();
void utils_restore_terminal();

#endif
