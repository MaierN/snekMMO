#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>

#include "../queue.h"
#include "../snake.h"

#define SERVER_MAX_CLIENTS 16

typedef struct {
    bool active;
    int clifd;
    snake_t snake;
    bool is_self;
} server_client_t;

server_client_t server_clients[SERVER_MAX_CLIENTS];
queue_t server_queue_in;

typedef struct {
    uint32_t slot;
    uint32_t size;
    uint8_t buf[];
} server_msg_t;

void server_init();
void server_start(int port);

#endif
