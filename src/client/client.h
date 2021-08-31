#ifndef CLIENT_H
#define CLIENT_H

#include "stdint.h"
#include "stddef.h"

typedef void (*client_message_callback_t)(uint8_t *, size_t, void *);
typedef void (*client_close_callback_t)(void *);

void client_handle_messages(int fd, client_message_callback_t callback, client_close_callback_t close_callback, void *arg);
void client_start(char *addr, int port);

#endif
