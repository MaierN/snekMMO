#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct queue_elt {
    struct queue_elt *to_head;
    struct queue_elt *to_tail;
    void *elt;
} queue_elt_t;

typedef struct {
    queue_elt_t *head;
    queue_elt_t *tail;
    pthread_mutex_t mutex;
} queue_t;

void queue_init(queue_t *this);
void queue_enqueue(queue_t *this, void *elt);
void *queue_dequeue(queue_t *this);
bool queue_empty(queue_t *this);

#endif
