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

#include "../queue.h"
#include "sgame.h"

#define SERVER_PORT 21337
#define SERVER_BUF_SIZE 1024

typedef struct {
    int clifd;
    int slot;
} server_thread_args;

static void err_check(int status, char *msg) {
    if (status == -1) {
        printf("%s (errno %d)\n", msg, errno);
        exit(1);
    }
}

static void *server_thread_run(void *vargp) {
    server_thread_args *args = (server_thread_args *)vargp;
    uint8_t buf[SERVER_BUF_SIZE];

    struct pollfd pfd;
	pfd.fd = args->clifd;
	pfd.events = POLLIN | POLLHUP | POLLRDNORM;
	pfd.revents = 0;
	while (true) {
        int status = poll(&pfd, 1, 100);
        err_check(status, "failed socket poll");
		if (status > 0) {
            if (recv(args->clifd, buf, sizeof(buf), MSG_DONTWAIT | MSG_PEEK) == 0) {
                close(args->clifd);
                printf("socket (%d) closed 1\n", args->clifd);
                sgame_remove_snake(args->slot);
                break;
            } else {
                ssize_t size = recv(args->clifd, buf, sizeof(buf), 0);
                if (size == -1) {
                    close(args->clifd);
                    printf("socket (%d) closed 2\n", args->clifd);
                    sgame_remove_snake(args->slot);
                    break;
                }
                //printf("message (%d) %ld bytes: %.*s\n", clifd, size, (int)size, buf);
                server_msg_t *msg = malloc(sizeof(server_msg_t) + size);
                msg->slot = args->slot;
                msg->size = size;
                memcpy(msg->buf, buf, size);
                queue_enqueue(&server_queue_in, msg);
            }
        }
    }

    return NULL;
}

void server_init() {
    queue_init(&server_queue_in);
    for (int i = 0; i < SERVER_MAX_CLIENTS; i++) {
        server_clients[i] = (server_client_t){0};
    }
}

void server_start() {
    printf("start server...\n");

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    err_check(sockfd, "failed socket creation");

    err_check(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)), "failed socket opt REUSEADDR");

    struct sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERVER_PORT);

    err_check(bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)), "failed socket bind");

    err_check(listen(sockfd, 64), "failed socket listen");

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
            err_check(close(args->clifd), "failed client close");
        } else {
            printf("accepted slot %d\n", i);

            args->slot = i;

            sgame_add_snake(i, args->clifd);

            pthread_t cli_thread_id;
            pthread_create(&cli_thread_id, NULL, server_thread_run, args);
        }
    }
}
