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

void snake_init(snake_t *this) {
    vector_init(&this->segments);
    point_t *seg1 = (point_t *)malloc(sizeof(point_t));
    seg1->x = 3;
    seg1->y = 1;
    point_t *seg2 = (point_t *)malloc(sizeof(point_t));
    seg2->x = 2;
    seg2->y = 1;
    point_t *seg3 = (point_t *)malloc(sizeof(point_t));
    seg3->x = 1;
    seg3->y = 1;
    vector_append(&this->segments, seg1);
    vector_append(&this->segments, seg2);
    vector_append(&this->segments, seg3);
    this->direction = SNAKE_DIRECTION_RIGHT;
    this->last_direction = SNAKE_DIRECTION_RIGHT;
    this->extend = false;
}

void snake_control_direction(snake_t *this, snake_direction_t direction) {
    if (!is_opposite_direction(this->last_direction, direction)) {
        this->direction = direction;
    }
}

void snake_step(snake_t *this) {
    if (this->extend) {
        void *new_segment = malloc(sizeof(point_t));
        vector_append(&this->segments, new_segment);
        this->extend = false;
    }
    for (int i = vector_size(&this->segments)-1; i > 0; i--) {
        point_t *curr = (point_t *)vector_get(&this->segments, i);
        point_t *last = (point_t *)vector_get(&this->segments, i-1);
        *curr = *last;
    }
    point_t *head = (point_t *)vector_get(&this->segments, 0);
    if (this->direction == SNAKE_DIRECTION_UP) head->y--;
    if (this->direction == SNAKE_DIRECTION_DOWN) head->y++;
    if (this->direction == SNAKE_DIRECTION_RIGHT) head->x++;
    if (this->direction == SNAKE_DIRECTION_LEFT) head->x--;
    this->last_direction = this->direction;
    //printf("new head: %d %d\n", head->x, head->y);
}

void snake_display(snake_t *this) {
    for (int i = vector_size(&this->segments)-1; i >= 0; i--) {
        point_t *curr = (point_t *)vector_get(&this->segments, i);
        display_write(curr->x, curr->y, "x");
    }
}

void snake_extend(snake_t *this) {
    this->extend = true;
}

bool snake_is_game_over(snake_t *this, int slot) {
    point_t *head = (point_t *)vector_get(&this->segments, 0);
    if (head->x == 0 || head->y == 0 || head->x == CONFIG_DISPLAY_WIDTH-1 || head->y == CONFIG_DISPLAY_HEIGHT-1) {
        return true;
    }
    for (int i = 0; i < SERVER_MAX_CLIENTS; i++) {
        if (!server_clients[i].active) continue;
        if (snake_is_on_point(&server_clients[i].snake, head, slot == i)) {
            //printf("found point on slot %d, slot was %d...\n", i, slot);
            return true;
        }
    }
    return false;
}

bool snake_is_on_point(snake_t *this, point_t *point, bool ignore_head) {
    for (int i = vector_size(&this->segments)-1; i >= ignore_head ? 1 : 0; i--) {
        point_t *curr = (point_t *)vector_get(&this->segments, i);
        if (point->x == curr->x && point->y == curr->y) {
            return true;
        }
    }
    return false;
}
