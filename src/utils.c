#include "utils.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios oldt, newt;

void utils_err_check(int status, char *msg) {
    if (status == -1) {
        printf("%s (errno %d)\n", msg, errno);
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
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}
