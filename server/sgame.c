
#include "sgame.h"

#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "../queue.h"
#include "server.h"
#include "../snake.h"
#include "../config.h"
#include "../point.h"
#include "../display.h"

pthread_mutex_t sgame_mutex;

point_t apple = {.x=8, .y=8};

static void *sgame_thread_run(void *vargp) {
    (void)vargp;
    while (true) {
        pthread_mutex_lock(&sgame_mutex);

        printf("game step...\n");
        while (!queue_empty(&server_queue_in)) {
            server_msg_t *msg = queue_dequeue(&server_queue_in);
            printf("sgame got message from %d:", msg->slot);
            for (uint32_t i = 0; i < msg->size; i++) {
                printf(" %d", msg->buf[i]);
            }
            printf("\n");
            if (msg->size == 0) continue;
            uint8_t dir = msg->buf[0];
            if (dir >= SNAKE_DIRECTION_N) continue;
            snake_control_direction(&server_clients[msg->slot].snake, dir);
        }

        for (int i = 0; i < SERVER_MAX_CLIENTS; i++) {
            if (!server_clients[i].active) continue;
            snake_step(&server_clients[i].snake);
        }

        bool to_game_over[SERVER_MAX_CLIENTS] = {0};

        for (int i = 0; i < SERVER_MAX_CLIENTS; i++) {
            if (!server_clients[i].active) continue;
            if (snake_is_game_over(&server_clients[i].snake, i)) {
                to_game_over[i] = true;
            }
        }

        for (int i = 0; i < SERVER_MAX_CLIENTS; i++) {
            if (to_game_over[i]) {
                server_clients[i].active = false;
                printf("GAME OVER %d\n", i);
                close(server_clients[i].clifd);
            }
        }

        for (int i = 0; i < SERVER_MAX_CLIENTS; i++) {
            if (!server_clients[i].active) continue;
            if (snake_is_on_point(&server_clients[i].snake, &apple, false)) {
                snake_extend(&server_clients[i].snake);
                for (int i = 0; i < 5; i++) {
                    printf("%d extending...\n", i);
                }

                bool apple_ok = false;

                while (!apple_ok) {
                    apple.x = 1 + rand() % (CONFIG_DISPLAY_WIDTH-2);
                    apple.y = 1 + rand() % (CONFIG_DISPLAY_HEIGHT-2);

                    apple_ok = true;

                    for (int j = 0; j < SERVER_MAX_CLIENTS; j++) {
                        if (!server_clients[j].active) continue;
                        if (snake_is_on_point(&server_clients[j].snake, &apple, false)) apple_ok = false;
                    }
                }
            }
        }

        for (int c = 0; c < CONFIG_DISPLAY_WIDTH; c++) {
            for (int r = 0; r < CONFIG_DISPLAY_HEIGHT; r++) {
                if (c == 0 || c == CONFIG_DISPLAY_WIDTH-1 || r == 0 || r == CONFIG_DISPLAY_HEIGHT-1) {
                    display_write(c, r, "#");
                } else {
                    point_t curr = {.x=c, .y=r};
                    bool is_snake = false;
                    for (int i = 0; i < SERVER_MAX_CLIENTS; i++) {
                        if (!server_clients[i].active) continue;
                        if (snake_is_on_point(&server_clients[i].snake, &curr, false)) is_snake = true;
                    }
                    if (is_snake) {
                        display_write(c, r, "x");
                    } else if (c == apple.x && r == apple.y) {
                        display_write(c, r, "o");
                    } else {
                        display_write(c, r, " ");
                    }
                }
            }
        }

        display_show(0, CONFIG_DISPLAY_HEIGHT);

        pthread_mutex_unlock(&sgame_mutex);

        usleep(1000 * 1000);
    }
    return NULL;
}

void sgame_start() {
    pthread_mutex_init(&sgame_mutex, NULL);
    pthread_t sgame_thread_id;
    pthread_create(&sgame_thread_id, NULL, sgame_thread_run, NULL);
}

void sgame_add_snake(int slot, int clifd) {
    pthread_mutex_lock(&sgame_mutex);
    server_clients[slot].active = true;
    server_clients[slot].clifd = clifd;
    snake_init(&server_clients[slot].snake);
    pthread_mutex_unlock(&sgame_mutex);
}

void sgame_remove_snake(int slot) {
    pthread_mutex_lock(&sgame_mutex);
    server_clients[slot].active = false;
    pthread_mutex_unlock(&sgame_mutex);
}
