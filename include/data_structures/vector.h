#ifndef VECTOR_H
#define VECTOR_H

#include "common_types.h"

typedef struct vector* Vector;

Vector vector_create(int size, DestroyFunc destroy_value);

int vector_size(Vector vec);

void vector_insert_last(Vector vec, Pointer value);

void vector_remove_last(Vector vec);

Pointer vector_get_at(Vector vec, int index);

void vector_set_at(Vector vec, int index, Pointer value);

// returns the old DestroyFunc
DestroyFunc vector_set_destroy_value(Vector vec, DestroyFunc destroy_value);

void vector_destroy(Vector vec);

// ! Modifies the order of the elements in exchange for an O(1) removal
// it swaps the the element at the given index and then calls remove_last
void vector_remove_at(Vector vec, int index);

#endif // VECTOR_H