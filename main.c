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

bool running = true;
int extend = 0;

int message = 0;

snake_t the_snake;
point_t apple = {.x=8, .y=8};

void disp_clear() {
    printf("\033[2J");
}

void disp_write(int x, int y, char* text) {
    printf("\033[%d;%dH%s", y+1, x+1, text);
}

void disp_show(int x, int y) {
    printf("\033[%d;%dH", y+1, x+1);
    fflush(stdout);
}

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

    disp_clear();

    pthread_t input_thread_id;
    pthread_create(&input_thread_id, NULL, input_thread_run, NULL);

    while (running) {
        snake_step(&the_snake);

        if (snake_is_game_over(&the_snake)) {
            running = false;
            message = 1;
            break;
        }

        if (snake_is_on_point(&the_snake, &apple)) {
            snake_extend(&the_snake);
            while (true) {
                apple.x = 1 + rand() % (CONFIG_DISPLAY_WIDTH-2);
                apple.y = 1 + rand() % (CONFIG_DISPLAY_HEIGHT-2);

                if (!snake_is_on_point(&the_snake, &apple)) {
                    break;
                }
            }
        }

        for (int c = 0; c < CONFIG_DISPLAY_WIDTH; c++) {
            for (int r = 0; r < CONFIG_DISPLAY_HEIGHT; r++) {
                if (c == 0 || c == CONFIG_DISPLAY_WIDTH-1 || r == 0 || r == CONFIG_DISPLAY_HEIGHT-1) {
                    disp_write(c, r, "#");
                } else {
                    point_t curr = {.x=c, .y=r};
                    if (snake_is_on_point(&the_snake, &curr)) {
                        disp_write(c, r, "x");
                    } else if (c == apple.x && r == apple.y) {
                        disp_write(c, r, "o");
                    } else {
                        disp_write(c, r, " ");
                    }
                }
            }
        }

        disp_show(0, CONFIG_DISPLAY_HEIGHT);
        usleep(200000);
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
