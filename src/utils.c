#include "utils.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

bool running = true;

struct termios oldt, newt;

void utils_err_check_no_exit(int status, char *msg) {
    if (status == -1) {
        fprintf(stderr, "%s (errno %d)\n", msg, errno);
    }
}

void utils_err_check(int status, char *msg) {
    if (status == -1) {
        fprintf(stderr, "%s (errno %d)\n", msg, errno);
        utils_restore_terminal();
        exit(1);
    }
}

void utils_init_terminal() {
    newt = oldt = (struct termios){0};

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
}

void utils_setup_terminal() {
    newt.c_lflag &= ~ICANON;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void utils_restore_terminal() {
    printf("\033[0m\n");
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}
