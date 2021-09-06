#ifndef SGAME_H
#define SGAME_H

#include "server.h"

void sgame_render_all();
void sgame_start();
void sgame_add_snake(int slot, server_thread_args *args);
void sgame_remove_snake(int slot);

#endif