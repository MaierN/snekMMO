#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>

#include "../data_structures/queue.h"
#include "../snake.h"

#define SERVER_MAX_CLIENTS 32

typedef struct {
    bool active;
    int clifd;
    snake_t snake;
    bool is_self;
    pthread_t thread_id;
} server_client_t;

typedef struct {
    int clifd;
    int slot;
} server_thread_args;

server_client_t server_clients[SERVER_MAX_CLIENTS];
queue_t server_queue_in;

typedef struct {
    uint32_t slot;
    uint32_t size;
    uint8_t buf[];
} server_msg_t;

void server_init();
void server_start(int port);
void *server_thread_run(void *vargp);

#endif
