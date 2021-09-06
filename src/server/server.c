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
#include <signal.h>

#include "../data_structures/queue.h"
#include "sgame.h"
#include "../utils.h"
#include "../client/client.h"
#include "../config.h"

static void server_message_callback(uint8_t *buf, size_t size, void *arg) {
    server_msg_t *msg = (server_msg_t *)malloc(sizeof(server_msg_t) + size);
    msg->slot = *(int *)arg;
    msg->size = size;
    memcpy(msg->buf, buf, size);
    queue_enqueue(&server_queue_in, msg);
}

static void server_close_callback(void *arg) {
    if (DEBUG) fprintf(stderr, "server close callback\n");
    sgame_remove_snake(*(int *)arg);
    sgame_render_all();
}

static void *server_input_thread_run(void *vargp) {
    pthread_t main_thread = *(pthread_t *)vargp;

    uint8_t char_buf[1];

    struct pollfd pfd;
    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN | POLLHUP;
    pfd.revents = 0;

    while (running) {
        if (poll(&pfd, 1, 100) && (pfd.revents & POLLIN)) {
            read(STDIN_FILENO, char_buf, 1);
            if (char_buf[0] == 'q') {
                running = false;
            }
        }
    }

    pthread_kill(main_thread, SIGALRM);

    if (DEBUG) fprintf(stderr, "server input thread terminated...\n");
    return NULL;
}

void *server_thread_run(void *vargp) {
    server_thread_args *args = (server_thread_args *)vargp;

    client_handle_messages(args->clifd, server_message_callback, server_close_callback, &args->slot);

    free(vargp);

    if (DEBUG) fprintf(stderr, "server network thread (%ld) terminated...\n", pthread_self());
    return NULL;
}

void server_init() {
    queue_init(&server_queue_in);
    for (int i = 0; i < SERVER_MAX_CLIENTS; i++) {
        server_clients[i] = (server_client_t){0};
    }
}

static void server_alrm_handler(int signum) {
    if (DEBUG) fprintf(stderr, "server thread interrupted with %d\n", signum);
}

void server_start(int port) {
    if (DEBUG) fprintf(stderr, "start server...\n");

    struct sigaction new_action;
    new_action = (struct sigaction){0};
    new_action.sa_handler = server_alrm_handler;
    sigaction(SIGALRM, &new_action, NULL);

    pthread_t self_thread = pthread_self();
    pthread_t input_thread_id;
    pthread_create(&input_thread_id, NULL, server_input_thread_run, &self_thread);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    utils_err_check(sockfd, "failed socket creation");

    utils_err_check(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)), "failed socket opt REUSEADDR");

    struct sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    utils_err_check(bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)), "failed socket bind");

    utils_err_check(listen(sockfd, 64), "failed socket listen");

    while (running) {
        struct sockaddr_in cliaddr = {0};
        unsigned int clilen = sizeof(cliaddr);

        if (DEBUG) fprintf(stderr, "waiting connection...\n");
        int clifd = accept(sockfd, (struct sockaddr *)&cliaddr, &clilen);
        if (clifd < 0) continue;

        if (DEBUG) fprintf(stderr, "connection! (%d)\n", clifd);

        int i = 0;
        for (; i < SERVER_MAX_CLIENTS; i++) {
            if (!server_clients[i].active) {
                break;
            }
        }
        if (i >= SERVER_MAX_CLIENTS) {
            if (DEBUG) fprintf(stderr, "refused\n");
            utils_err_check_no_exit(close(clifd), "failed client close");
        } else {
            if (DEBUG) fprintf(stderr, "accepted slot %d\n", i);

            server_thread_args *args = (server_thread_args *)malloc(sizeof(server_thread_args));
            args->clifd = clifd;
            args->slot = i;

            sgame_add_snake(i, args);

            sgame_render_all();
        }
    }

    pthread_join(input_thread_id, NULL);

    if (DEBUG) fprintf(stderr, "main server thread end...\n");

    for (int i = 0; i < SERVER_MAX_CLIENTS; i++) {
        if (server_clients[i].active) {
            close(server_clients[i].clifd);
            pthread_join(server_clients[i].thread_id, NULL);
        }
    }
}
