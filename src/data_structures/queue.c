#include "queue.h"

#include <stdlib.h>

void queue_init(queue_t *this) {
    this->head = NULL;
    this->tail = NULL;
    pthread_mutex_init(&this->mutex, NULL);
}

void queue_enqueue(queue_t *this, void *elt) {
    pthread_mutex_lock(&this->mutex);

    queue_elt_t *queue_elt = malloc(sizeof(queue_elt_t));
    queue_elt->to_head = this->tail;
    queue_elt->to_tail = NULL;
    queue_elt->elt = elt;

    if (this->tail != NULL) {
        this->tail->to_tail = queue_elt;
    }
    this->tail = queue_elt;
    if (this->head == NULL) {
        this->head = queue_elt;
    }

    pthread_mutex_unlock(&this->mutex);
}

void *queue_dequeue(queue_t *this) {
    pthread_mutex_lock(&this->mutex);

    if (this->head->to_tail == NULL) {
        this->tail = NULL;
    }
    void *elt = this->head->elt;
    queue_elt_t *new_head = this->head->to_tail;
    free(this->head);
    this->head = new_head;
    if (this->head != NULL) {
        this->head->to_head = NULL;
    }

    pthread_mutex_unlock(&this->mutex);

    return elt;
}

bool queue_empty(queue_t *this) {
    pthread_mutex_lock(&this->mutex);
    bool res = this->head == NULL;
    pthread_mutex_unlock(&this->mutex);
    return res;
}
