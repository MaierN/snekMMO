#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>

typedef struct {
    void **array;
    size_t size;
    size_t reserved_size;
} vector_t;

void vector_init(vector_t *this);
void vector_append(vector_t *this, void *elt);
size_t vector_size(vector_t *this);
void *vector_get(vector_t *this, size_t index);

#endif
