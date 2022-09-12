// Reference: https: d-michail.github.io/assets/teaching/data-structures/013_VectorImplementation.en.pdf

#ifndef _DYNAMICINTARRAY_H
#define _DYNAMICINTARRAY_H

typedef struct uint32_vector* uint32_vector ;

uint32_vector uint32_vector_new();
void uint32_vector_free(uint32_vector v);
uint32_t uint32_vector_get(uint32_vector v, int i); //each value is a segment
void uint32_vector_put(uint32_vector v, int i, uint32_t value); //i == index
void uint32_vector_addhi(uint32_vector v, uint32_t value);
void uint32_vector_add_at(uint32_vector v, int i, uint32_t value);
uint32_t uint32_vector_remove_at(uint32_vector v, int i);
int uint32_vector_is_empty(uint32_vector v);
int uint32_vector_length(uint32_vector v);
void uint32_vector_clear(uint32_vector v);

#endif
