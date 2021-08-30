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

#define SERVER_PORT 21337
#define SERVER_BUF_SIZE 1024

static void err_check(int status, char *msg) {
    if (status == -1) {
        printf("%s (errno %d)\n", msg, errno);
        exit(1);
    }
}

static void *server_thread_run(void *vargp) {
    int clifd = *(int *)vargp;
    uint8_t buf[SERVER_BUF_SIZE];

    struct pollfd pfd;
	pfd.fd = clifd;
	pfd.events = POLLIN | POLLHUP | POLLRDNORM;
	pfd.revents = 0;
	while (true) {
        int status = poll(&pfd, 1, 100);
        err_check(status, "failed socket poll");
		if (status > 0) {
            if (recv(clifd, buf, sizeof(buf), MSG_DONTWAIT | MSG_PEEK) == 0) {
                close(clifd);
                printf("socket (%d) closed\n", clifd);
                break;
            } else {
                ssize_t size = recv(clifd, buf, sizeof(buf), 0);
                //printf("message (%d) %ld bytes: %.*s\n", clifd, size, (int)size, buf);
                server_msg_t *msg = malloc(sizeof(server_msg_t) + size);
                msg->cliid = clifd;
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
        int *clifd = malloc(sizeof(int));
        *clifd = accept(sockfd, (struct sockaddr *)&cliaddr, &clilen);

        printf("connection! (%d)\n", *clifd);

        pthread_t cli_thread_id;
        pthread_create(&cli_thread_id, NULL, server_thread_run, clifd);
    }
}
