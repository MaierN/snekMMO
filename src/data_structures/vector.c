#include "vector.h"

#include <stdlib.h>

#define VECTOR_INITIAL_SIZE 2
#define VECTOR_SIZE_FACTOR 2

void vector_init(vector_t *this) {
    this->array = malloc(VECTOR_INITIAL_SIZE * sizeof(void *));
    this->size = 0;
    this->reserved_size = VECTOR_INITIAL_SIZE;
    pthread_mutex_init(&this->mutex, NULL);
}

void vector_append(vector_t *this, void *elt) {
    pthread_mutex_lock(&this->mutex);
    if (this->size == this->reserved_size) {
        this->reserved_size *= VECTOR_SIZE_FACTOR;
        this->array = realloc(this->array, this->reserved_size * sizeof(void *));
    }
    this->array[this->size] = elt;
    this->size++;
    pthread_mutex_unlock(&this->mutex);
}

size_t vector_size(vector_t *this) {
    pthread_mutex_lock(&this->mutex);
    size_t res = this->size;
    pthread_mutex_unlock(&this->mutex);
    return res;
}

void *vector_get(vector_t *this, size_t index) {
    pthread_mutex_lock(&this->mutex);
    void *res = this->array[index];
    pthread_mutex_unlock(&this->mutex);
    return res;
}

void vector_clear(vector_t *this) {
    pthread_mutex_lock(&this->mutex);
    this->size = 0;
    pthread_mutex_unlock(&this->mutex);
}

void vector_delete(vector_t *this) {
    pthread_mutex_lock(&this->mutex);
    free(this->array);
    pthread_mutex_unlock(&this->mutex);
}
