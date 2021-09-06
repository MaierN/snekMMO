#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

#include "config.h"
#include "snake.h"
#include "server/server.h"
#include "server/sgame.h"
#include "display.h"
#include "client/client.h"
#include "utils.h"

#define BUFSIZE 128

int main(int argc, char **argv) {
    srand(time(NULL));

    int opt;
    char port_s[BUFSIZE+1] = {0};
    char addr[BUFSIZE+1] = {0};
    bool has_port = false;
    bool has_addr = false;
    while ((opt = getopt(argc, argv, "i:p:")) != -1) {
        switch (opt) {
            case 'i':
                has_addr = true;
                snprintf(addr, BUFSIZE, "%s", optarg);
                break;
            case 'p':
                has_port = true;
                snprintf(port_s, BUFSIZE, "%s", optarg);
                break;
        }
    }

    if (!has_port) {
        has_port = true;
        strcpy(port_s, "21338");
    }

    if (!has_port) {
        fprintf(stderr, "Usage: %s [-i address] [-p port]\n", argv[0]);
        fprintf(stderr, "Examples: %s -i localhost -p 21337 #connects client to specified server\n", argv[0]);
        fprintf(stderr, "          %s -p 21337              #starts server on specified port\n", argv[0]);
        exit(0);
    }

    utils_init_terminal();

    int port = strtol(port_s, NULL, 10);

    utils_setup_terminal();
    display_clear();
    if (has_addr) {
        client_start(addr, port);
    } else {
        server_init();
        sgame_start();
        server_start(port);
        sgame_stop();
    }
    utils_restore_terminal();

    return 0;
}
