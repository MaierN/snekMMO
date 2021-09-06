#include "queue.h"

#include <stdlib.h>

void queue_init(queue_t *self) {
    self->head = NULL;
    self->tail = NULL;
    pthread_mutex_init(&self->mutex, NULL);
}

void queue_enqueue(queue_t *self, void *elt) {
    pthread_mutex_lock(&self->mutex);

    queue_elt_t *queue_elt = malloc(sizeof(queue_elt_t));
    queue_elt->to_head = self->tail;
    queue_elt->to_tail = NULL;
    queue_elt->elt = elt;

    if (self->tail != NULL) {
        self->tail->to_tail = queue_elt;
    }
    self->tail = queue_elt;
    if (self->head == NULL) {
        self->head = queue_elt;
    }

    pthread_mutex_unlock(&self->mutex);
}

void *queue_dequeue(queue_t *self) {
    pthread_mutex_lock(&self->mutex);

    if (self->head->to_tail == NULL) {
        self->tail = NULL;
    }
    void *elt = self->head->elt;
    queue_elt_t *new_head = self->head->to_tail;
    free(self->head);
    self->head = new_head;
    if (self->head != NULL) {
        self->head->to_head = NULL;
    }

    pthread_mutex_unlock(&self->mutex);

    return elt;
}

bool queue_empty(queue_t *self) {
    pthread_mutex_lock(&self->mutex);
    bool res = self->head == NULL;
    pthread_mutex_unlock(&self->mutex);
    return res;
}
