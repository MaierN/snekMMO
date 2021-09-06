#ifndef SNAKE_H
#define SNAKE_H

#include <stdbool.h>

#include "data_structures/vector.h"
#include "point.h"

typedef enum {
    SNAKE_DIRECTION_UP,
    SNAKE_DIRECTION_DOWN,
    SNAKE_DIRECTION_RIGHT,
    SNAKE_DIRECTION_LEFT,
    SNAKE_DIRECTION_N
} snake_direction_t;

typedef struct {
    vector_t segments;
    snake_direction_t direction;
    snake_direction_t last_direction;
    bool extend;
} snake_t;

void snake_init(snake_t *self, bool empty);
void snake_control_direction(snake_t *self, snake_direction_t direction);
void snake_step(snake_t *self);
void snake_extend(snake_t *self);
bool snake_is_game_over(snake_t *self, int slot);
bool snake_is_on_point(snake_t *self, point_t *point, bool ignore_head);
void snake_delete(snake_t *self);

#endif
