/* umInstructions.h
 * HW06: um
 * Lucas Maley and Colby Cho
 * Interface of the Universal Machine instruction set.
 */

 #ifndef UM_INSTR_H_INCLUDED
 #define UM_INSTR_H_INCLUDED

 #include <stdlib.h>
 #include <stdio.h>
 #include <stdint.h>
 #include <string.h>
 #include <sys/stat.h>
 // #include "seq.h"
 #include "assert.h"
 #include "bitpack.h"

typedef uint32_t* value_type;

 struct uint32_vector {
     uint32_t* array;
     int size;
     int capacity;
 };

 struct _vector {
     value_type* array;
     int size;
     int capacity;
 };


 typedef uint32_t Um_instruction;
 typedef struct uint32_vector* uint32_vector;
 typedef struct _vector* vector;
 
 struct UM {
     Um_instruction registers[8]; //
     vector segmented_memory; //
     uint32_vector segment_lengths; //
     uint32_vector unmappedID; //
     uint32_t program_counter; 
 };
 
 typedef struct UM *UM;


 #endif