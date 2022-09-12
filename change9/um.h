/* um.h
 * HW06: um
 * Lucas Maley and Colby Cho
 * Interface of the Universal Machine architecture.
 */

#ifndef UM_H_INCLUDED
#define UM_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include "assert.h"
#include "bitpack.h"
#include "mem.h"

typedef struct uint32_vector* uint32_vector;
typedef struct _vector* vector;
typedef uint32_t* value_type;
typedef uint32_t Um_instruction;

struct UM {
    Um_instruction registers[8];
    vector segmented_memory;
    uint32_vector segment_lengths;
    uint32_vector unmappedID;
    uint32_t program_counter;
};

typedef struct UM *UM;


UM initialize_UM();
void load_mem(char *filename, UM um);


#endif