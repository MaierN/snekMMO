#ifndef DISPLAY_H
#define DISPLAY_H

#include "server/server.h"

void display_clear();
void display_write(int x, int y, char* text, int color);
void display_show(int x, int y);
void display_render_game(server_client_t *clients, point_t *apple);

#endif
