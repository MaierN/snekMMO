#include "vector.h"

#include <stdlib.h>

#define VECTOR_INITIAL_SIZE 2
#define VECTOR_SIZE_FACTOR 2

void vector_init(vector_t *this) {
    this->array = malloc(VECTOR_INITIAL_SIZE * sizeof(void *));
    this->size = 0;
    this->reserved_size = VECTOR_INITIAL_SIZE;
}

void vector_append(vector_t *this, void *elt) {
    if (this->size == this->reserved_size) {
        this->reserved_size *= VECTOR_SIZE_FACTOR;
        this->array = realloc(this->array, this->reserved_size * sizeof(void *));
    }
    this->array[this->size] = elt;
    this->size++;
}

size_t vector_size(vector_t *this) {
    return this->size;
}

void *vector_get(vector_t *this, size_t index) {
    return this->array[index];
}

void vector_clear(vector_t *this) {
    this->size = 0;
}

void vector_delete(vector_t *this) {
    free(this->array);
}
