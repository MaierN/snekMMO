#include "vector.h"

#include <stdlib.h>

#define VECTOR_INITIAL_SIZE 2
#define VECTOR_SIZE_FACTOR 2

void vector_init(vector_t *self) {
    self->array = malloc(VECTOR_INITIAL_SIZE * sizeof(void *));
    self->size = 0;
    self->reserved_size = VECTOR_INITIAL_SIZE;
    pthread_mutex_init(&self->mutex, NULL);
}

void vector_append(vector_t *self, void *elt) {
    pthread_mutex_lock(&self->mutex);
    if (self->size == self->reserved_size) {
        self->reserved_size *= VECTOR_SIZE_FACTOR;
        self->array = realloc(self->array, self->reserved_size * sizeof(void *));
    }
    self->array[self->size] = elt;
    self->size++;
    pthread_mutex_unlock(&self->mutex);
}

size_t vector_size(vector_t *self) {
    pthread_mutex_lock(&self->mutex);
    size_t res = self->size;
    pthread_mutex_unlock(&self->mutex);
    return res;
}

void *vector_get(vector_t *self, size_t index) {
    pthread_mutex_lock(&self->mutex);
    void *res = self->array[index];
    pthread_mutex_unlock(&self->mutex);
    return res;
}

void vector_clear(vector_t *self) {
    pthread_mutex_lock(&self->mutex);
    self->size = 0;
    pthread_mutex_unlock(&self->mutex);
}

void vector_delete(vector_t *self) {
    pthread_mutex_lock(&self->mutex);
    free(self->array);
    pthread_mutex_unlock(&self->mutex);
}
