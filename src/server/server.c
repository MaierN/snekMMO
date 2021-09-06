#include "server.h"

#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>

#include "../data_structures/queue.h"
#include "sgame.h"
#include "../utils.h"
#include "../client/client.h"

typedef struct {
    int clifd;
    int slot;
} server_thread_args;

static void server_message_callback(uint8_t *buf, size_t size, void *arg) {
    server_msg_t *msg = malloc(sizeof(server_msg_t) + size);
    msg->slot = *(int *)arg;
    msg->size = size;
    memcpy(msg->buf, buf, size);
    queue_enqueue(&server_queue_in, msg);
}

static void server_close_callback(void *arg) {
    sgame_remove_snake(*(int *)arg);
    sgame_render_all();
}

static void *server_thread_run(void *vargp) {
    server_thread_args *args = (server_thread_args *)vargp;

    client_handle_messages(args->clifd, server_message_callback, server_close_callback, &args->slot);

    return NULL;
}

void server_init() {
    queue_init(&server_queue_in);
    for (int i = 0; i < SERVER_MAX_CLIENTS; i++) {
        server_clients[i] = (server_client_t){0};
    }
}

void server_start(int port) {
    printf("start server...\n");

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    utils_err_check(sockfd, "failed socket creation");

    utils_err_check(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)), "failed socket opt REUSEADDR");

    struct sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    utils_err_check(bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)), "failed socket bind");

    utils_err_check(listen(sockfd, 64), "failed socket listen");

    while (true) {
        struct sockaddr_in cliaddr = {0};
        unsigned int clilen = sizeof(cliaddr);

        printf("waiting connection...\n");
        server_thread_args *args = malloc(sizeof(server_thread_args));
        args->clifd = accept(sockfd, (struct sockaddr *)&cliaddr, &clilen);

        printf("connection! (%d)\n", args->clifd);

        int i = 0;
        for (; i < SERVER_MAX_CLIENTS; i++) {
            if (!server_clients[i].active) {
                break;
            }
        }
        if (i >= SERVER_MAX_CLIENTS) {
            printf("refused\n");
            utils_err_check_no_exit(close(args->clifd), "failed client close");
        } else {
            printf("accepted slot %d\n", i);

            args->slot = i;

            sgame_add_snake(i, args->clifd);

            pthread_t cli_thread_id;
            pthread_create(&cli_thread_id, NULL, server_thread_run, args);

            sgame_render_all();
        }
    }
}
