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
 #include <sys/stat.h>
 #include "seq.h"
 #include "assert.h"
 #include "bitpack.h"
 #include "um.h"
 #include "dynamicArray.h"


 void execute_instr(UM um);


 #endif