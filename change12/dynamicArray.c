#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include "dynamicArray.h"
#define INITIAL_CAPACITY 64

#define min(x,y) (((x)<(y))?(x):(y))

struct _vector {
    value_type* array;
    int size;
    int capacity;
};

vector vector_new() 
{
    vector v = (vector) malloc(sizeof(struct _vector));
    if (v == NULL) {
        fprintf(stderr, "Not enough memory!");
        abort();
    }
    v->size = 0;
    v->capacity = INITIAL_CAPACITY;
    v->array = (value_type*) malloc(sizeof(value_type) * v->capacity);
    if (v->array == NULL) {
        fprintf(stderr, "Not enough memory!");
        abort();
    }
    return v;
}

void vector_free(vector v) 
{
    assert(v);
    free(v->array);
    free(v);
}

static
void vector_double_capacity(vector v) 
{
    assert(v);
    int new_capacity = 2 * v->capacity;
    value_type* new_array = (value_type*) malloc(sizeof(value_type)*new_capacity);
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
void vector_half_capacity(vector v)
{
    assert(v);
    if (v->capacity <= INITIAL_CAPACITY) {
        return;
    }
    int new_capacity = v->capacity / 2;
    value_type* new_array = (value_type*) 
    malloc(sizeof(value_type)*new_capacity);
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

void vector_addhi(vector v, value_type value) 
{
    assert(v);
    if (v->size >= v->capacity) {
        vector_double_capacity(v);
    }
    v->array[v->size++] = value;
}

value_type vector_get(vector v, int i) 
{
    assert(v);
    if (i < 0 || i >= v->size) {
        fprintf(stderr, "Out of index!");
        abort();
    }
    return v->array[i];
}
    
void vector_put(vector v, int i, value_type value) 
{
    assert(v);
    if (i < 0 || i >= v->size) {
        fprintf(stderr, "Out of index!");
        abort();
    }
    v->array[i] = value;
}

void vector_add_at(vector v, int i, value_type value) 
{
    assert(v);
    if (i < 0 || i >= v->size) {
        fprintf(stderr, "Out of index!");
        abort();
    }
    if (v->size >= v->capacity) {
        vector_double_capacity(v);
    }
    for(int j = v->size; j>i; j--) {
        v->array[j] = v->array[j-1];
    }
    v->array[i] = value;
    v->size++;
}

value_type vector_remove_at(vector v, int i) 
{
    assert(v);
    if (i < 0 || i >= v->size) {
        fprintf(stderr, "Out of index!");
        abort();
    }
    value_type ret = v->array[i];
    for(int j = i+1; j < v->size; j++) {
        v->array[j-1] = v->array[j];
    }
    v->size--;
    if (4 * v->size < v->capacity) {
        vector_half_capacity(v);
    }
    return ret;
}

int vector_is_empty(vector v) 
{
    assert(v);
    return v->size == 0;
}

int vector_length(vector v) 
{
    assert(v);
    return v->size;
}

void vector_clear(vector v) 
{
    assert(v);
    v->size = 0;
    while (v->capacity > INITIAL_CAPACITY) {
        vector_half_capacity(v);
    }
}
