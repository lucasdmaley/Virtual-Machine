/* um.c
 * HW06: um
 * Lucas Maley and Colby Cho
 * Implementation of the Universal Machine architecture.
 */
 
#include "um.h"

/* 
 * initialize_UM
 * Description:
 * - Creates an instance of a Universal Machine 
 * Parameters:
 * - None
 * Effects:
 * - None
 * Returns:
 * - An initialized UM without a loaded segment 0
 */
UM initialize_UM() 
{
    UM um = malloc(sizeof(struct UM));
    assert(um != NULL);
    
    for (int i = 0; i < 8; i++) {
        um->registers[i] = 0;
    }
    
    um->segmented_memory = vector_new();
    um->segment_lengths = Seq_new(1);
    um->unmappedID = Seq_new(0);
    um->program_counter = 0;
    
    return um;
}

/* 
 * load_mem
 * Description:
 * - Loads the instructions given in the file and stores it in segment 0 of 
 *   the Universal Machine passed in
 * Parameters:
 * - The filename of the file containing UM instructions (char *filename)
 * - Pointer to a Universal Machine struct (UM um)
 * Effects:
 * - $m[0] is loaded with the instructions given in the file
 * Returns:
 * - None
 */
void load_mem(char *filename, UM um) 
{
    FILE *input = fopen(filename, "r");
    assert(input != NULL);
    
    struct stat buf;
    stat(filename, &buf);
    
    /* Filesize in bytes */
    off_t filesize = buf.st_size;
    int num_instructions = filesize / 4;
    
    Um_instruction *segment_zero = malloc(num_instructions * 4);
    assert(segment_zero != NULL);
    
    /* Packs each 4 characters in file into an instruction and stores that 
       in segment zero */
    for (int i = 0; i < num_instructions; i++) {
        Um_instruction tempinstruction = 0;
        for (int j = 0; j < 4; j++) {
            tempinstruction = Bitpack_newu(tempinstruction, 8, (24 - (8 * j)),
                                           fgetc(input));
        }
        segment_zero[i] = tempinstruction;
    }
    
    /* Updates the UM struct */
    uint32_t *num_instructions_p = malloc(sizeof(num_instructions));
    *num_instructions_p = num_instructions;
    vector_addhi(um->segmented_memory, segment_zero);
    Seq_addhi(um->segment_lengths, num_instructions_p);

    fclose(input);
}
