#include "vector.h"
#include <assert.h>
#include "sys_utils.h"

#define VEC_MIN_CAPACITY 8

typedef struct {
    Pointer value;
} *VectorNode;

struct vector {
    VectorNode array;
    int size;
    int capacity;
    DestroyFunc destroy_value;
};

Vector vector_create(int size, DestroyFunc destroy_value){
    assert(size >= 0);
    Vector vec = safe_malloc(sizeof(*vec));

    vec->size = 0;
    vec->destroy_value = destroy_value;

    vec->capacity = size > VEC_MIN_CAPACITY ? size : VEC_MIN_CAPACITY;
    vec->array = safe_malloc(vec->capacity * sizeof(*vec->array));

    return vec;
}

int vector_size(Vector vec){
    assert(vec != NULL);
    return vec->size;
}

void vector_insert_last(Vector vec, Pointer value){
    assert(vec != NULL);

    if(vec->size == vec->capacity){
        vec->capacity *= 2;
        vec->array = safe_realloc(vec->array, vec->capacity * sizeof(*vec->array));
    }

    vec->array[vec->size++].value = value;
}

void vector_remove_last(Vector vec){
    assert(vec != NULL && vec->size > 0);

    if(vec->destroy_value != NULL){
        vec->destroy_value(vec->array[vec->size - 1].value);
    }

    vec->size--;

    // resize if 3/4 of vector are empty
    if(vec->size * 4 < vec->capacity && vec->capacity > 2 * VEC_MIN_CAPACITY){
        vec->capacity /= 2;
        vec->array = safe_realloc(vec->array, vec->capacity * sizeof(*vec->array));
    }
}

Pointer vector_get_at(Vector vec, int index){
    assert(vec != NULL && index >= 0 && index < vec->size);

    return vec->array[index].value;
}

void vector_set_at(Vector vec, int index, Pointer value){
    assert(vec != NULL && index >= 0 && index < vec->size);

    if(vec->destroy_value != NULL){
        vec->destroy_value(vec->array[index].value);
    }

    vec->array[index].value = value;
}

// returns the old DestroyFunc
DestroyFunc vector_set_destroy_value(Vector vec, DestroyFunc destroy_value){
    assert(vec != NULL);

    DestroyFunc old = vec->destroy_value;
    vec->destroy_value = destroy_value;

    return old;
}

void vector_destroy(Vector vec){
    assert(vec != NULL);

    if(vec->destroy_value != NULL){
        for(int i = 0 ; i < vec->size ; i++){
            vec->destroy_value(vec->array[i].value);
        }
    }

    free(vec->array);
    free(vec);
}