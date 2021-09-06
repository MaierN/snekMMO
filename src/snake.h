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

void snake_init(snake_t *this, bool empty);
void snake_control_direction(snake_t *this, snake_direction_t direction);
void snake_step(snake_t *this);
void snake_extend(snake_t *this);
bool snake_is_game_over(snake_t *this, int slot);
bool snake_is_on_point(snake_t *this, point_t *point, bool ignore_head);
void snake_delete(snake_t *this);

#endif
