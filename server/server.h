#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>

#include "../queue.h"

queue_t server_queue_in;

typedef struct {
    uint32_t cliid;
    uint32_t size;
    uint8_t buf[];
} server_msg_t;

void server_init();
void server_start();

#endif
