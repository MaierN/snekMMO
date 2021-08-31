
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
#include "../utils.h"
#include "../vector.h"

pthread_mutex_t sgame_mutex;

point_t apple = {.x=8, .y=8};

void sgame_render_all() {
    pthread_mutex_lock(&sgame_mutex);

    display_render_game(server_clients, &apple);

    uint8_t *buf = (uint8_t *)malloc(1);
    size_t size;

    for (int i = 0; i < SERVER_MAX_CLIENTS; i++) {
        if (!server_clients[i].active) continue;

        size = sizeof(point_t) * vector_size(&server_clients[i].snake.segments) + 2;
        buf = realloc(buf, size);

        for (int j = 0; j < SERVER_MAX_CLIENTS; j++) {
            if (!server_clients[j].active) continue;

            buf[0] = 0;
            buf[1] = j;

            for (size_t s = 0; s < vector_size(&server_clients[i].snake.segments); s++) {
                *(((point_t *)(buf + 2)) + s) = *(point_t *)vector_get(&server_clients[i].snake.segments, s);
            }

            utils_err_check_no_exit(write(server_clients[j].clifd, buf, size), "failed write");
        }
    }

    size = sizeof(point_t) + 1;
    buf = realloc(buf, size);

    for (int i = 0; i < SERVER_MAX_CLIENTS; i++) {
        if (!server_clients[i].active) continue;

        buf[0] = 1;
        *((point_t *)(buf + 1)) = apple;

        utils_err_check_no_exit(write(server_clients[i].clifd, buf, size), "failed write");
    }

    pthread_mutex_unlock(&sgame_mutex);

    free(buf);
}

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

        pthread_mutex_unlock(&sgame_mutex);

        sgame_render_all();

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
