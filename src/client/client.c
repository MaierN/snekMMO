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
#include "../server/server.h"
#include "../display.h"
#include "../vector.h"

#define CLIENT_BUF_RECV_SIZE 1024
#define CLIENT_BUF_SEND_SIZE 16

bool running = true;

server_client_t client_snakes[SERVER_MAX_CLIENTS];

static void client_message_callback(uint8_t *buf, size_t size, void *arg) {
    (void)arg;

    //printf("client received message of size %ld\n", size);

    if (buf[0] == 0) {
        uint8_t r_slot = buf[1];
        bool is_self = buf[2];

        int n = (size - 3) / sizeof(point_t);

        vector_clear(&client_snakes[r_slot].snake.segments);
        for (int i = 0; i < n; i++) {
            point_t *point = (point_t *)malloc(sizeof(point_t));
            *point = *((point_t *)(buf + 3) + i);
            vector_append(&client_snakes[r_slot].snake.segments, point);
        }
        client_snakes[r_slot].active = true;
        client_snakes[r_slot].is_self = is_self;
    } else if (buf[0] == 1) {
        point_t *apple = ((point_t *)(buf + 1));

        display_render_game(client_snakes, apple);

        for (int i = 0; i < SERVER_MAX_CLIENTS; i++) {
            client_snakes[i].active = false;
        }
    }
}

static void client_close_callback(void *arg) {
    (void)arg;
}

static void *client_input_thread_run(void *vargp) {
    int sockfd = *(int*)vargp;

    uint8_t buf[CLIENT_BUF_SEND_SIZE];

    while (running) {
        *(uint16_t *)buf = 3;
        buf[2] = 0xff;

        char c = getchar();
        if (c == 27) {
            getchar();
            c = getchar();
            if (c == 65) buf[2] = SNAKE_DIRECTION_UP;
            if (c == 66) buf[2] = SNAKE_DIRECTION_DOWN;
            if (c == 67) buf[2] = SNAKE_DIRECTION_RIGHT;
            if (c == 68) buf[2] = SNAKE_DIRECTION_LEFT;
        }

        if (c == 'w') buf[2] = SNAKE_DIRECTION_UP;
        if (c == 's') buf[2] = SNAKE_DIRECTION_DOWN;
        if (c == 'd') buf[2] = SNAKE_DIRECTION_RIGHT;
        if (c == 'a') buf[2] = SNAKE_DIRECTION_LEFT;

        if (c == 'q') {
            running = false;
        }

        if (buf[2] != 0xff) {
            utils_err_check(write(sockfd, buf, 3), "failed write");
        }
    }
    return NULL;
}

void client_handle_messages(int fd, client_message_callback_t callback, client_close_callback_t close_callback, void *arg) {
    uint8_t buf[CLIENT_BUF_RECV_SIZE];

    int received_size = 0;
    uint8_t *received_data = (uint8_t *)malloc(1);

    struct pollfd pfd;
	pfd.fd = fd;
	pfd.events = POLLIN | POLLHUP | POLLRDNORM;
	pfd.revents = 0;
	while (true) {
        int status = poll(&pfd, 1, 100);
        utils_err_check_no_exit(status, "failed socket poll");
		if (status > 0) {
            if (recv(fd, buf, sizeof(buf), MSG_DONTWAIT | MSG_PEEK) == 0) {
                close(fd);
                printf("socket (%d) closed 1\n", fd);
                close_callback(arg);
                break;
            } else {
                ssize_t size = recv(fd, buf, sizeof(buf), 0);
                if (size == -1) {
                    close(fd);
                    printf("socket (%d) closed 2\n", fd);
                    close_callback(arg);
                    break;
                }

                received_data = realloc(received_data, received_size + size);

                for (int i = 0; i < size; i++) {
                    received_data[i + received_size] = buf[i];
                }
                received_size += size;

                while (received_size >= 2) {
                    uint16_t expected_size = *(uint16_t *)received_data;
                    if (expected_size < 2) break;
                    if (received_size >= expected_size) {
                        callback(received_data + 2, expected_size - 2, arg);

                        received_size -= expected_size;
                        for (int i = 0; i < received_size; i++) {
                            received_data[i] = received_data[i + expected_size];
                        }
                    } else {
                        break;
                    }
                }
            }
        }
    }
}

void client_start(char *addr, int port) {
    //printf("client start (addr: %s, port: %d)...\n", addr, port);

    for (int i = 0; i < SERVER_MAX_CLIENTS; i++) {
        client_snakes[i] = (server_client_t){0};
        snake_init(&client_snakes[i].snake);
    }

    struct hostent *he = gethostbyname(addr);
    if (he == NULL || !(struct in_addr **)he->h_addr_list[0]) {
        printf("host not found\n");
        utils_restore_terminal();
        exit(1);
    }
    struct in_addr **addr_list = (struct in_addr **)he->h_addr_list;
    //printf("ip: %s\n", inet_ntoa(*addr_list[0]));

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    utils_err_check(sockfd, "failed socket creation");

    struct sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = addr_list[0]->s_addr;
    servaddr.sin_port = htons(port);

    utils_err_check(connect(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)), "failed socket connect");

    pthread_t input_thread_id;
    pthread_create(&input_thread_id, NULL, client_input_thread_run, &sockfd);

    client_handle_messages(sockfd, client_message_callback, client_close_callback, NULL);

}
