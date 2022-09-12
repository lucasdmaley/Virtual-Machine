#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include "dynamicIntArray.h"
#define INITIAL_CAPACITY 64

#define min(x,y) (((x)<(y))?(x):(y))

struct uint32_vector {
    uint32_t* array;
    int size;
    int capacity;
};

uint32_vector uint32_vector_new() 
{
    uint32_vector v = (uint32_vector) malloc(sizeof(struct uint32_vector));
    if (v == NULL) {
        fprintf(stderr, "Not enough memory!");
        abort();
    }
    v->size = 0;
    v->capacity = INITIAL_CAPACITY;
    v->array = (uint32_t*) malloc(sizeof(int) * v->capacity);
    if (v->array == NULL) {
        fprintf(stderr, "Not enough memory!");
        abort();
    }
    return v;
}

void uint32_vector_free(uint32_vector v) 
{
    assert(v);
    free(v->array);
    free(v);
}

static
void uint32_vector_double_capacity(uint32_vector v) 
{
    assert(v);
    int new_capacity = 2 * v->capacity;
    uint32_t* new_array = (uint32_t*) malloc(sizeof(int)*new_capacity);
    if (new_array == NULL) {
        fprintf(stderr, "Not enough memory!");
        abort();
    }
    for(int i = 0; i < v->size; i++) {
        new_array[i] = v->array[i];
    }
    free(v->array);
    v->array = new_array;
    v->capacity = new_capacity;
}

static
void uint32_vector_half_capacity(uint32_vector v)
{
    assert(v);
    if (v->capacity <= INITIAL_CAPACITY) {
        return;
    }
    int new_capacity = v->capacity / 2;
    uint32_t* new_array = (uint32_t*) 
    malloc(sizeof(int)*new_capacity);
    if (new_array == NULL) {
        fprintf(stderr, "Not enough memory!");
        abort();
    }
    for(int i = 0; i < min(v->size, new_capacity); i++) {
        new_array[i] = v->array[i];
    }
    free(v->array);
    v->array = new_array;
    v->capacity = new_capacity;
    v->size = min(v->size, new_capacity);
}

void uint32_vector_addhi(uint32_vector v, uint32_t value) 
{
    assert(v);
    if (v->size >= v->capacity) {
        uint32_vector_double_capacity(v);
    }
    v->array[v->size++] = value;
}

uint32_t uint32_vector_get(uint32_vector v, int i) 
{
    assert(v);
    if (i < 0 || i >= v->size) {
        fprintf(stderr, "Out of index!");
        abort();
    }
    return v->array[i];
}
    
void uint32_vector_put(uint32_vector v, int i, uint32_t value) 
{
    assert(v);
    if (i < 0 || i >= v->size) {
        fprintf(stderr, "Out of index!");
        abort();
    }
    v->array[i] = value;
}

void uint32_vector_add_at(uint32_vector v, int i, uint32_t value) 
{
    assert(v);
    if (i < 0 || i >= v->size) {
        fprintf(stderr, "Out of index!");
        abort();
    }
    if (v->size >= v->capacity) {
        uint32_vector_double_capacity(v);
    }
    for(int j = v->size; j>i; j--) {
        v->array[j] = v->array[j-1];
    }
    v->array[i] = value;
    v->size++;
}

uint32_t uint32_vector_remove_at(uint32_vector v, int i) 
{
    assert(v);
    if (i < 0 || i >= v->size) {
        fprintf(stderr, "Out of index!");
        abort();
    }
    int ret = v->array[i];
    for(int j = i+1; j < v->size; j++) {
        v->array[j-1] = v->array[j];
    }
    v->size--;
    if (4 * v->size < v->capacity) {
        uint32_vector_half_capacity(v);
    }
    return ret;
}

int uint32_vector_is_empty(uint32_vector v) 
{
    assert(v);
    return v->size == 0;
}

int uint32_vector_length(uint32_vector v) 
{
    assert(v);
    return v->size;
}

void uint32_vector_clear(uint32_vector v) 
{
    assert(v);
    v->size = 0;
    while (v->capacity > INITIAL_CAPACITY) {
        uint32_vector_half_capacity(v);
    }
}
