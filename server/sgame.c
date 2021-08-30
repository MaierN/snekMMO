
#include "sgame.h"

#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>

#include "../queue.h"
#include "server.h"

static void *sgame_thread_run(void *vargp) {
    (void)vargp;
    while (true) {
        printf("game running...\n");
        while (!queue_empty(&server_queue_in)) {
            server_msg_t *msg = queue_dequeue(&server_queue_in);
            printf("sgame got message from %d...\n", msg->cliid);
        }
        usleep(1000 * 1000);
    }
    return NULL;
}

void sgame_start() {
    pthread_t sgame_thread_id;
    pthread_create(&sgame_thread_id, NULL, sgame_thread_run, NULL);
}
