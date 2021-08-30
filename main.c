#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <time.h>

#include "config.h"
#include "snake.h"
#include "server/server.h"
#include "server/sgame.h"
#include "display.h"

bool running = true;
int extend = 0;

int message = 0;

snake_t the_snake;

void *input_thread_run(void *vargp) {
    (void)vargp;
    while (running) {

        char c = getchar();
        if (c == 27) {
            getchar();
            c = getchar();
            if (c == 65) snake_control_direction(&the_snake, SNAKE_DIRECTION_UP);
            if (c == 66) snake_control_direction(&the_snake, SNAKE_DIRECTION_DOWN);
            if (c == 67) snake_control_direction(&the_snake, SNAKE_DIRECTION_RIGHT);
            if (c == 68) snake_control_direction(&the_snake, SNAKE_DIRECTION_LEFT);
        }
        if (c == 'q') {
            running = false;
            message = 2;
            break;
        }

        fflush(stdout);
    }
    return NULL;
}

int main() {
    srand(time(NULL));

    sgame_start();

    server_start();
    exit(0);

    snake_init(&the_snake);

    static struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    newt.c_lflag &= ~ICANON;
    newt.c_lflag &= ~ECHO;
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);

    display_clear();

    pthread_t input_thread_id;
    pthread_create(&input_thread_id, NULL, input_thread_run, NULL);

    while (running) {
    }

    pthread_cancel(input_thread_id);

    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);

    if (message == 1) {
        printf("\nGAME OVER\n");
    } else if (message == 2) {
        printf("\nEXIT\n");
    }

    return 0;
}
