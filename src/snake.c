#include "snake.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "display.h"
#include "config.h"
#include "server/server.h"

static bool is_opposite_direction(snake_direction_t a, snake_direction_t b) {
    if (b < a) {
        snake_direction_t tmp = a;
        a = b;
        b = tmp;
    }
    return (a == SNAKE_DIRECTION_RIGHT && b == SNAKE_DIRECTION_LEFT) || (a == SNAKE_DIRECTION_UP && b == SNAKE_DIRECTION_DOWN);
}

void snake_init(snake_t *self, bool empty) {
    vector_init(&self->segments);
    if (!empty) {
        point_t *seg1 = (point_t *)malloc(sizeof(point_t));
        seg1->x = 3;
        seg1->y = 1;
        point_t *seg2 = (point_t *)malloc(sizeof(point_t));
        seg2->x = 2;
        seg2->y = 1;
        point_t *seg3 = (point_t *)malloc(sizeof(point_t));
        seg3->x = 1;
        seg3->y = 1;
        vector_append(&self->segments, seg1);
        vector_append(&self->segments, seg2);
        vector_append(&self->segments, seg3);
    }
    self->direction = SNAKE_DIRECTION_RIGHT;
    self->last_direction = SNAKE_DIRECTION_RIGHT;
    self->extend = false;
}

void snake_control_direction(snake_t *self, snake_direction_t direction) {
    if (!is_opposite_direction(self->last_direction, direction)) {
        self->direction = direction;
    }
}

void snake_step(snake_t *self) {
    if (self->extend) {
        void *new_segment = malloc(sizeof(point_t));
        vector_append(&self->segments, new_segment);
        self->extend = false;
    }
    for (int i = vector_size(&self->segments)-1; i > 0; i--) {
        point_t *curr = (point_t *)vector_get(&self->segments, i);
        point_t *last = (point_t *)vector_get(&self->segments, i-1);
        *curr = *last;
    }
    point_t *head = (point_t *)vector_get(&self->segments, 0);
    if (self->direction == SNAKE_DIRECTION_UP) head->y--;
    if (self->direction == SNAKE_DIRECTION_DOWN) head->y++;
    if (self->direction == SNAKE_DIRECTION_RIGHT) head->x++;
    if (self->direction == SNAKE_DIRECTION_LEFT) head->x--;
    self->last_direction = self->direction;
}

void snake_extend(snake_t *self) {
    self->extend = true;
}

bool snake_is_game_over(snake_t *self, int slot) {
    point_t *head = (point_t *)vector_get(&self->segments, 0);
    if (head->x == 0 || head->y == 0 || head->x == CONFIG_DISPLAY_WIDTH-1 || head->y == CONFIG_DISPLAY_HEIGHT-1) {
        return true;
    }
    for (int i = 0; i < SERVER_MAX_CLIENTS; i++) {
        if (!server_clients[i].active) continue;
        if (snake_is_on_point(&server_clients[i].snake, head, slot == i)) {
            return true;
        }
    }
    return false;
}

bool snake_is_on_point(snake_t *self, point_t *point, bool ignore_head) {
    for (int i = vector_size(&self->segments)-1; i >= ignore_head ? 1 : 0; i--) {
        point_t *curr = (point_t *)vector_get(&self->segments, i);
        if (point->x == curr->x && point->y == curr->y) {
            return true;
        }
    }
    return false;
}

void snake_delete(snake_t *self) {
    for (size_t i = 0; i < vector_size(&self->segments); i++) {
        free(vector_get(&self->segments, i));
    }
    vector_delete(&self->segments);
}
