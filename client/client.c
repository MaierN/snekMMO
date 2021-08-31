#include "client.h"

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <netdb.h>

#include "../snake.h"
#include "../utils.h"

#define CLIENT_BUF_RECV_SIZE 1024
#define CLIENT_BUF_SEND_SIZE 16

bool running = true;

static void *client_input_thread_run(void *vargp) {
    int sockfd = *(int*)vargp;

    uint8_t buf[CLIENT_BUF_SEND_SIZE];

    while (running) {
        buf[0] = 0xff;

        char c = getchar();
        if (c == 27) {
            getchar();
            c = getchar();
            if (c == 65) buf[0] = SNAKE_DIRECTION_UP;
            if (c == 66) buf[0] = SNAKE_DIRECTION_DOWN;
            if (c == 67) buf[0] = SNAKE_DIRECTION_RIGHT;
            if (c == 68) buf[0] = SNAKE_DIRECTION_LEFT;
        }
        if (c == 'q') {
            running = false;
        }

        if (buf[0] != 0xff) {
            utils_err_check(write(sockfd, buf, 1), "failed write");
        }
    }
    return NULL;
}

void client_start(char *addr, int port) {
    printf("client start (addr: %s, port: %d)...\n", addr, port);

    struct hostent *he = gethostbyname(addr);
    if (he == NULL || !(struct in_addr **)he->h_addr_list[0]) {
        printf("host not found\n");
        utils_restore_terminal();
        exit(1);
    }
    struct in_addr **addr_list = (struct in_addr **)he->h_addr_list;
    printf("ip: %s\n", inet_ntoa(*addr_list[0]));

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    utils_err_check(sockfd, "failed socket creation");

    struct sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = addr_list[0]->s_addr;
    servaddr.sin_port = htons(port);

    utils_err_check(connect(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)), "failed socket connect");

    pthread_t input_thread_id;
    pthread_create(&input_thread_id, NULL, client_input_thread_run, &sockfd);

    uint8_t buf[CLIENT_BUF_RECV_SIZE];

    struct pollfd pfd;
	pfd.fd = sockfd;
	pfd.events = POLLIN | POLLHUP | POLLRDNORM;
	pfd.revents = 0;
	while (true) {
        int status = poll(&pfd, 1, 100);
        utils_err_check(status, "failed socket poll");
		if (status > 0) {
            if (!running || recv(sockfd, buf, sizeof(buf), MSG_DONTWAIT | MSG_PEEK) == 0) {
                running = false;
                close(sockfd);
                printf("socket (%d) closed 1\n", sockfd);
                break;
            } else {
                ssize_t size = recv(sockfd, buf, sizeof(buf), 0);
                if (size == -1) {
                    running = false;
                    close(sockfd);
                    printf("socket (%d) closed 2\n", sockfd);
                    break;
                }
                printf("message (%d) %ld bytes: %.*s\n", sockfd, size, (int)size, buf);
                //server_msg_t *msg = malloc(sizeof(server_msg_t) + size);
                //msg->slot = args->slot;
                //msg->size = size;
                //memcpy(msg->buf, buf, size);
                //queue_enqueue(&server_queue_in, msg);
            }
        }
    }
}
