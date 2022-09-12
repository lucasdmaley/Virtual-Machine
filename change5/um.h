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
#include "seq.h"
#include "assert.h"
#include "bitpack.h"
#include "mem.h"
#include "dynamicArray.h"

typedef uint32_t Um_instruction;

struct UM {
    Um_instruction registers[8];
    vector segmented_memory;
    Seq_T segment_lengths;
    Seq_T unmappedID;
    uint32_t program_counter;
};

typedef struct UM *UM;


UM initialize_UM();
void load_mem(char *filename, UM um);


#endif