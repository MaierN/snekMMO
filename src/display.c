#include "display.h"

#include <stdio.h>

#include "config.h"

void display_clear() {
    printf("\033[2J");
}

void display_write(int x, int y, char* text) {
    x += 40;
    printf("\033[%d;%dH%s", y+1, x+1, text);
}

void display_show(int x, int y) {
    x += 40;
    printf("\033[%d;%dH", y+1, x+1);
    fflush(stdout);
}

void display_render_game(server_client_t *clients, point_t *apple) {
    for (int c = 0; c < CONFIG_DISPLAY_WIDTH; c++) {
        for (int r = 0; r < CONFIG_DISPLAY_HEIGHT; r++) {
            if (c == 0 || c == CONFIG_DISPLAY_WIDTH-1 || r == 0 || r == CONFIG_DISPLAY_HEIGHT-1) {
                display_write(c, r, "#");
            } else {
                point_t curr = {.x=c, .y=r};
                bool is_snake = false;
                bool is_self = false;
                for (int i = 0; i < SERVER_MAX_CLIENTS; i++) {
                    if (!clients[i].active) continue;
                    if (snake_is_on_point(&clients[i].snake, &curr, false)) {
                        is_snake = true;
                        if (clients[i].is_self) is_self = true;
                    }
                }
                if (is_snake) {
                    display_write(c, r, is_self ? "+" : "x");
                } else if (c == apple->x && r == apple->y) {
                    display_write(c, r, "o");
                } else {
                    display_write(c, r, " ");
                }
            }
        }
    }

    display_show(0, CONFIG_DISPLAY_HEIGHT);
}
