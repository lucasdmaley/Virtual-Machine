// Reference: https: d-michail.github.io/assets/teaching/data-structures/013_VectorImplementation.en.pdf

#ifndef _VECTOR_H
#define _VECTOR_H

typedef struct _vector* vector;
typedef uint32_t* value_type;

vector vector_new();
void vector_free(vector v);
value_type vector_get(vector v, int i); //each value is a segment
void vector_put(vector v, int i, value_type value); //i == index
void vector_addhi(vector v, value_type value);
void vector_add_at(vector v, int i, value_type value);
value_type vector_remove_at(vector v, int i);
int vector_is_empty(vector v);
int vector_length(vector v);
void vector_clear(vector v);

#endif
