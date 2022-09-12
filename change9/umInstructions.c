/* umInstructions.c
 * HW06: um
 * Lucas Maley and Colby Cho
 * Implementation of the Universal Machine instruction set.
 */
 
 #include "umInstructions.h"
 #include <time.h>
 
 #define INITIAL_CAPACITY 64

 #define min(x,y) (((x)<(y))?(x):(y))
 
 
 typedef enum Um_opcode {
         CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
         NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
 } Um_opcode;

 /********************* uint32_vector interface ******************************/

static inline uint32_vector uint32_vector_new();
static inline void uint32_vector_free(uint32_vector v);
static inline uint32_t uint32_vector_get(uint32_vector v, int i); //each value is a segment
static inline void uint32_vector_put(uint32_vector v, int i, uint32_t value); //i == index
static inline void uint32_vector_addhi(uint32_vector v, uint32_t value);
static inline void uint32_vector_add_at(uint32_vector v, int i, uint32_t 
                                         value);
static inline uint32_t uint32_vector_remove_at(uint32_vector v, int i);
static inline int uint32_vector_is_empty(uint32_vector v);
static inline int uint32_vector_length(uint32_vector v);
static inline void uint32_vector_clear(uint32_vector v);
 
 /********************* vector interface *************************************/

static inline vector vector_new();
static inline void vector_free(vector v);
static inline value_type vector_get(vector v, int i); //each value is a segment
static inline void vector_put(vector v, int i, value_type value); //i == index
static inline void vector_addhi(vector v, value_type value);
static inline void vector_add_at(vector v, int i, value_type value);
static inline value_type vector_remove_at(vector v, int i);
static inline int vector_is_empty(vector v);
static inline int vector_length(vector v);
static inline void vector_clear(vector v);
  
/********************* Private Function Declarations *************************/
/* UM Instructions */
static inline void conditional_move(Um_instruction inst, 
                                    Um_instruction *registers);
static inline void segmented_load(Um_instruction inst, Um_instruction *registers, vector 
                    segmented_memory, uint32_vector segment_lengths);
static inline void segmented_store(Um_instruction inst, Um_instruction *registers, vector 
                     segmented_memory, uint32_vector segment_lengths);

static inline void addition(Um_instruction inst, Um_instruction 
                            *registers);
static inline void multiplication(Um_instruction inst, 
                                  Um_instruction *registers);
static inline void division(Um_instruction inst, Um_instruction *registers);
static inline void bitwise_NAND(Um_instruction inst, 
                                Um_instruction *registers);
static inline void halt(UM um);
static inline void map_segment(Um_instruction inst, UM um);
static inline void unmap_segment(Um_instruction inst, UM um);
static inline void output (Um_instruction inst, Um_instruction *registers);
static inline void input(Um_instruction inst, Um_instruction *registers);
static inline void load_program(Um_instruction inst, Um_instruction *registers, vector 
                  segmented_memory, uint32_vector segment_lengths, uint32_t* 
                  program_counter);
static inline void load_value(Um_instruction inst, Um_instruction *registers);

/* Helper Function */
static inline uint32_t get_reg_i(Um_instruction instruction, char character);
static inline uint64_t bp_get_u(uint64_t word, unsigned width, unsigned lsb);

/************************** Function Definitions *****************************/

/************************** um.c *****************************/
/* um.c
 * HW06: um
 * Lucas Maley and Colby Cho
 * Implementation of the Universal Machine architecture.
 */

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
    um->segment_lengths = uint32_vector_new();
    um->unmappedID = uint32_vector_new();
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
    vector_addhi(um->segmented_memory, segment_zero);
    uint32_vector_addhi(um->segment_lengths, num_instructions);

    fclose(input);
}

/************************** end of um.c *****************************/




/* 
 * execute_instr
 * Description:
 * - Executes the execution cycle until the halt instruction is reached. 
 *   Assumes UM passed in has been initialized with a valid set of 
 *   instructions
 * Parameters:
 * - Pointer to an initialized universal machine struct (UM um)
 * Returns:
 * - None
 */
void execute_instr(UM um)
{
    register uint16_t op_code = 0;
    
    // uint64_t total_count = 0;
    // uint64_t conmov_count = 0;
    // uint64_t segload_count = 0;
    // uint64_t segstore_count = 0;
    // uint64_t add_count = 0;
    // uint64_t mult_count = 0;
    // uint64_t div_count = 0;
    // uint64_t nand_count = 0;
    // uint64_t halt_count = 0;
    // uint64_t map_count = 0;
    // uint64_t unmap_count = 0;
    // uint64_t output_count = 0;
    // uint64_t input_count = 0;
    // uint64_t loadprog_count = 0;
    // uint64_t loadval_count = 0;
    while (op_code != 7) {
        // total_count++;
        register Um_instruction curr_instruction = 
        ((Um_instruction *)vector_get(um->segmented_memory, 0))
                                                         [um->program_counter];
        op_code = bp_get_u(curr_instruction, 4, 28);
        
        switch(op_code) {
            case 0: 
                conditional_move(curr_instruction, um->registers);
                //conmov_count++;
                break;
            case 1: 
                segmented_load(curr_instruction, um->registers, 
                               um->segmented_memory, um->segment_lengths);
                //segload_count++;
                break;
            case 2: 
                segmented_store(curr_instruction, um->registers, 
                               um->segmented_memory, um->segment_lengths);
                //segstore_count++;
                break;
            case 3: 
                addition(curr_instruction, um->registers);
                //add_count++;
                break;
            case 4: 
                multiplication(curr_instruction, um->registers);
                // mult_count++;
                break;
            case 5: 
                division(curr_instruction, um->registers);
                // div_count++;
                break;
            case 6: 
                bitwise_NAND(curr_instruction, um->registers);
                // nand_count++;
                break;
            case 7: 
                halt(um);
                // halt_count++;
                break;
            case 8: 
                map_segment(curr_instruction, um);
                // map_count++;
                break;
            case 9: 
                unmap_segment(curr_instruction, um);
                // unmap_count++;
                break;
            case 10: 
                output(curr_instruction, um->registers);
                // output_count++;
                break;
            case 11: 
                input(curr_instruction, um->registers);
                // input_count++;
                break;
            case 12:
                load_program(curr_instruction, um->registers, 
                    um->segmented_memory, um->segment_lengths, 
                    &um->program_counter);
                // loadprog_count++;
                um->program_counter--;
                break;
            case 13:
                load_value(curr_instruction, um->registers);
                // loadval_count++;
                break;
            default: 
                /* Failure mode: opcode out of range */
                fprintf(stderr, "Invalid opcode\n");
                exit(EXIT_FAILURE);
                break;
        }
        um->program_counter++;
    }
    // float proportion = 0;
    // proportion = (float)conmov_count / (float)total_count * 100;
    // printf("time spent:\n");
    // printf("Conditional Move: %f percent\n", proportion);
    // 
    // proportion = (float)segload_count / (float)total_count * 100;
    // printf("Segmented Load: %f percent\n", proportion);
    // 
    // proportion = (float)segstore_count / (float)total_count * 100;
    // printf("Conditional Move: %f percent\n", proportion);
    // 
    // proportion = (float)add_count / (float)total_count * 100;
    // printf("Addition: %f percent\n", proportion);
    // 
    // proportion = (float)mult_count / (float)total_count * 100;
    // printf("Multiplication: %f percent\n", proportion);
    // 
    // proportion = (float)div_count / (float)total_count * 100;
    // printf("Division: %f percent\n", proportion);
    // 
    // proportion = (float)nand_count / (float)total_count * 100;
    // printf("Bitwise NAND: %f percent\n", proportion);
    // 
    // proportion = (float)halt_count / (float)total_count * 100;
    // printf("Halt: %f percent\n", proportion);
    // 
    // proportion = (float)map_count / (float)total_count * 100;
    // printf("Map Segment: %f percent\n", proportion);
    // 
    // proportion = (float)unmap_count / (float)total_count * 100;
    // printf("Unmap Segment: %f percent\n", proportion);
    // 
    // proportion = (float)output_count / (float)total_count * 100;
    // printf("Output: %f percent\n", proportion);
    // 
    // proportion = (float)input_count / (float)total_count * 100;
    // printf("Input: %f percent\n", proportion);
    // 
    // proportion = (float)loadprog_count / (float)total_count * 100;
    // printf("Load Program: %f percent\n", proportion);
    // 
    // proportion = (float)loadval_count / (float)total_count * 100;
    // printf("Load Value: %f percent\n", proportion);
}

/*****************************************************************************
 *                           Instruction Set Functions                       *
 *****************************************************************************/

/* 
 * load_value
 * Description:
 * - Executes the load value instruction
 * Parameters:
 * - The load value instruction (Um_instruction instruction)
 * - Pointer to the registers of the UM (Um_instruction *registers)
 * Effects:
 * - Sets $r[A] to the value denoted by the value bits in instruction
 * Returns:
 * - None
 */
void load_value(Um_instruction instruction, Um_instruction *registers)
{
    uint32_t a = bp_get_u(instruction, 3, 25);
    registers[a] = bp_get_u(instruction, 25, 0);
}

/* 
 * halt
 * Description:
 * - Prepares the halting of the UM by freeing the dynamically allocated 
 *   memory stored in the UM
 * Parameters:
 * - Pointer to an initialized universal machine struct (UM um)
 * Effects:
 * - Frees dynamically allocated memory stored in the UM
 * Returns:
 * - None
 */
void halt(UM um)
{
    /* Frees segments and length values */
    // for (int i = 0; i < vector_length(um->segmented_memory); i++) {
    //     if (((Um_instruction *)vector_get(um->segmented_memory, i)) != NULL) { 
    //         free(((Um_instruction *)vector_get(um->segmented_memory, i)));
    //         free(((uint32_t *)int_vector_get(um->segment_lengths, i)));
    //     }
    // }
    int segmented_mem_len = vector_length(um->segmented_memory);
    for (int i = 0; i < segmented_mem_len; i++) {
        free(vector_get(um->segmented_memory, i));
    }
    vector_free(um->segmented_memory);
    uint32_vector_free(um->segment_lengths);
    /* deleted a free meant for segmented memory as a sequence */
    /* Frees IDs of unmapped segments */
    // for (int i = 0; i < int_vector_length(um->unmappedID); i++) {
    //     free(((uint32_t *)int_vector_get(um->unmappedID, i)));
    // }
    uint32_vector_free(um->unmappedID);
}

/* 
 * output
 * Description:
 * - Outputs the value stored in $r[C] to standard output
 * Parameters:
 * - The output instruction (Um_instruction instruction)
 * - Pointer to the registers of the UM (Um_instruction *registers)
 * Effects:
 * - If the value in $r[C] is a valid ASCII character, outputs that to 
 *   standard output. Otherwise, exits the program.
 * Returns:
 * - None
 */
void output(Um_instruction instruction, Um_instruction *registers)
{
    uint32_t c = registers[get_reg_i(instruction, 'c')];
    if (c > 255) {
        /* Failure mode: unchecked runtime error */
        fprintf(stderr, "Register c not within bounds\n");
        exit(EXIT_FAILURE);
    }
    putc(c, stdout);
}

/* 
 * addition
 * Description:
 * - Adds the values of $r[B] with $r[C] and mods by 2^32, storing 
 *   the result in $r[A]
 * Parameters:
 * - The addition instruction (Um_instruction instruction)
 * - Pointer to the registers of the UM (Um_instruction *registers)
 * Effects:
 * - Updates $r[A] with the value of $r[B] + $r[C] % 2^32
 * Returns:
 * - None
 */
void addition(Um_instruction inst, Um_instruction *registers) 
{
   uint64_t twopow32 = 4294967296; /* 2^32 */
   registers[get_reg_i(inst, 'a')] = (registers[get_reg_i(inst, 'b')] + 
                                      registers[get_reg_i(inst, 'c')]) % 
                                      twopow32;
}

/* 
 * multiplication
 * Description:
 * - Multiplies the values of $r[B] with $r[C] and mods by 2^32, storing 
 *   the result in $r[A]
 * Parameters:
 * - The multiplication instruction (Um_instruction instruction)
 * - Pointer to the registers of the UM (Um_instruction *registers)
 * Effects:
 * - Updates $r[A] with the value of $r[B] * $r[C] % 2^32
 * Returns:
 * - None
 */
void multiplication(Um_instruction inst, Um_instruction *registers) 
{
    uint64_t twopow32 = 4294967296; /* 2^32 */
    registers[get_reg_i(inst, 'a')] = (registers[get_reg_i(inst, 'b')] * 
                                       registers[get_reg_i(inst, 'c')]) % 
                                       twopow32;
}

/* 
 * divisionn
 * Description:
 * - Divides the values of $r[B] with $r[C], floors the result, and 
 *   stores the result in $r[A]
 * Parameters:
 * - The division instruction (Um_instruction instruction)
 * - Pointer to the registers of the UM (Um_instruction *registers)
 * Effects:
 * - Updates $r[A] with the value of floor($r[B] / $r[C])
 * Returns:
 * - None
 */
void division(Um_instruction inst, Um_instruction *registers) 
{
    if (registers[get_reg_i(inst, 'c')] == 0) {
        fprintf(stderr, "Cannot divide by zero.\n");
        exit(EXIT_FAILURE); /* Failure mode */
    }
   
    registers[get_reg_i(inst, 'a')] = (registers[get_reg_i(inst, 'b')] / 
                                       registers[get_reg_i(inst, 'c')]);
}

/* 
 * bitwise_NAND
 * Description:
 * - "Bitwise and"s the values of $r[B] with $r[C], nots the result, and 
 *   stores the result in $r[A]
 * Parameters:
 * - The bitwise NAND instruction (Um_instruction instruction)
 * - Pointer to the registers of the UM (Um_instruction *registers)
 * Effects:
 * - Updates $r[A] with the value of not($r[B] & $r[C])
 * Returns:
 * - None
 */
void bitwise_NAND(Um_instruction inst, Um_instruction *registers)
{
    registers[get_reg_i(inst, 'a')] = ~(registers[get_reg_i(inst, 'b')] & 
                                        registers[get_reg_i(inst, 'c')]);
}

/* 
 * conditional_move
 * Description:
 * - if $r[C] is not equal to 0, stores $r[B] in $r[A]
 * Parameters:
 * - The conditional move instruction (Um_instruction instruction)
 * - Pointer to the registers of the UM (Um_instruction *registers)
 * Effects:
 * - Updates $r[A] with the value of $r[B] if $r[C] is not equal to 0
 * Returns:
 * - None
 */
void conditional_move(Um_instruction inst, Um_instruction *registers)
{
    if (registers[get_reg_i(inst, 'c')] != 0) {
        registers[get_reg_i(inst, 'a')] = registers[get_reg_i(inst, 'b')];
    }
}

/* 
 * input
 * Description:
 * - Takes in a character from standard input and stores it in $r[C]
 * Parameters:
 * - The input instruction (Um_instruction instruction)
 * - Pointer to the registers of the UM (Um_instruction *registers)
 * Effects:
 * - Updates the value of $r[C] with the character given in on standard input.
 *   If end of input signalled, every bit in $r[C] is loaded with 1s.
 * Returns:
 * - None
 */
void input(Um_instruction inst, Um_instruction *registers)
{
    int inputval = fgetc(stdin);
    
    /* Contract Violation */
    if (inputval < -1 || inputval > 255) { /* EOF == -1 */
        fprintf(stderr, "Contract Violation: Must input value between 0 and");
        fprintf(stderr, " 255\n");
        exit(EXIT_FAILURE);
    }
    
    if (inputval != EOF) {
        registers[get_reg_i(inst, 'c')] = inputval;
    } else {
        registers[get_reg_i(inst, 'c')] = ~0;
    }
}

/* 
 * map_segment
 * Description:
 * - Creates a new segment with $r[C] instructions, each initialized to 0
 * Parameters:
 * - The map segment instruction (Um_instruction instruction)
 * - Pointer to an initialized universal machine struct (UM um)
 * Effects:
 * - Creates a new segment with $r[C] instructions, each initialized to 0, 
 *   The bit pattern is stored in $r[B]. The new segment is stored in 
 *   segmented memory at $m[$r[B]]
 * Returns:
 * - None
 */
void map_segment(Um_instruction inst, UM um)
{
    /* Declares and initializes a new segment on the heap */
    int num_words = um->registers[get_reg_i(inst, 'c')];

    Um_instruction *new_segment = malloc(num_words * 4);
    assert(new_segment != NULL);
    
    for (int i = 0; i < num_words; i++) {
        new_segment[i] = 0;
    }
    
    /* Check if any segments have been unmapped whose IDs can be reused */
    if (uint32_vector_length(um->unmappedID) != 0) {
        /* Maps segment with ID of previously unmapped segment */
        uint32_t reusableID = uint32_vector_get(um->unmappedID, 
                              uint32_vector_length(um->unmappedID) - 1);
        uint32_vector_remove_at(um->unmappedID, uint32_vector_length(um->unmappedID) - 1);
        vector_put(um->segmented_memory, reusableID, new_segment);
        uint32_vector_put(um->segment_lengths, reusableID, num_words);
        um->registers[get_reg_i(inst, 'b')] = reusableID;
    } else {
        /* Maps segment with new ID */
        vector_addhi(um->segmented_memory, new_segment);
        uint32_vector_addhi(um->segment_lengths, num_words);
        um->registers[get_reg_i(inst, 'b')] = vector_length(um->segmented_memory) - 1;
    }
}

/* 
 * unmap_segment
 * Description:
 * - Unmaps segment $m[$r[C]] instructions, adds $r[C] to unmapped ID sequence
 * Parameters:
 * - The unmap segment instruction (Um_instruction instruction)
 * - Pointer to an initialized universal machine struct (UM um)
 * Effects:
 * - Frees memory associated with segment $r[C], adds $r[C] to unmapped ID 
 *   sequence
 * Returns:
 * - None
 */
void unmap_segment(Um_instruction inst, UM um)
{
    uint32_t unmappedID = um->registers[get_reg_i(inst, 'c')];
    
    if (unmappedID == 0 || 
        unmappedID >= (uint32_t)vector_length(um->segmented_memory)) {
        fprintf(stderr, "Can't unmap segment 0 or non-mapped segments\n");
        exit(EXIT_FAILURE); /* Failure mode */
    }
    
    /* Unmaps the segment */
    free(vector_get(um->segmented_memory, unmappedID));
    vector_put(um->segmented_memory, unmappedID, NULL);
    
    uint32_vector_put(um->segment_lengths, unmappedID, 0);
    
    /* Adds the ID of the unmapped segment to the associated vector */
    // uint32_t *unmappedID_p = malloc(sizeof(unmappedID));
    // assert(unmappedID_p != NULL);
    // *unmappedID_p = unmappedID;
    
    uint32_vector_addhi(um->unmappedID, unmappedID);
}

/* 
 * segmented_load
 * Description:
 * - Loads the value from $m[$r[B]$r[C]] into $r[A]
 * Parameters:
 * - The segmented load instruction (Um_instruction instruction)
 * - Pointer to an initialized universal machine struct (UM um)
 * Effects:
 * - $r[A] is updated with the value of $m[$r[B]$r[C]]
 * Returns:
 * - None
 */
void segmented_load(Um_instruction inst, Um_instruction *registers, vector 
                    segmented_memory, uint32_vector segment_lengths)
{
    /* Store value of registers b and c */
    uint32_t bv = registers[get_reg_i(inst, 'b')];
    uint32_t cv = registers[get_reg_i(inst, 'c')];
    
    if (bv >= (uint32_t)vector_length(segmented_memory) ||
        (Um_instruction *)vector_get(segmented_memory, bv) == 0) {
        fprintf(stderr, "Trying to load unmapped segment\n");
        exit(EXIT_FAILURE); /* Failure mode */
    }
    
    uint32_t num_instructions = uint32_vector_get(segment_lengths, bv);

    if (cv > num_instructions) {
        fprintf(stderr, "Trying to access instruction out of bounds ");
        fprintf(stderr, "of mapped segment\n");
        exit(EXIT_FAILURE); /* Failure mode */
    }
    
    Um_instruction inst_at_rbrc = 
    ((Um_instruction *)vector_get(segmented_memory, bv))[cv];

    registers[get_reg_i(inst, 'a')] = inst_at_rbrc;
}

/* 
 * segmented_store
 * Description:
 * - Loads the value from $r[C] into $m[$r[A]$r[B]]
 * Parameters:
 * - The segmented store instruction (Um_instruction instruction)
 * - Pointer to an initialized universal machine struct (UM um)
 * Effects:
 * - $m[$r[A]$r[B]] is updated with the value of $r[C]
 * Returns:
 * - None
 */
void segmented_store(Um_instruction inst, Um_instruction *registers, vector 
                    segmented_memory, uint32_vector segment_lengths)
{
    /* Store value of registers a, b, and c */
    uint32_t av = registers[get_reg_i(inst, 'a')];
    uint32_t bv = registers[get_reg_i(inst, 'b')];
    uint32_t cv = registers[get_reg_i(inst, 'c')];
    
    if (av >= (uint32_t)vector_length(segmented_memory) ||
        (Um_instruction *)vector_get(segmented_memory, av) == NULL) {
        fprintf(stderr, "Trying to store in unmapped segment\n");
        exit(EXIT_FAILURE); /* Failure mode */
    }
    
    uint32_t num_instructions = uint32_vector_get(segment_lengths, av);
    
    if (bv > num_instructions) {
        fprintf(stderr, "Trying to access instruction out of bounds ");
        fprintf(stderr, "of mapped segment\n");
        exit(EXIT_FAILURE); /* Failure mode */
    }

    ((Um_instruction *)vector_get(segmented_memory, av))[bv] = cv;
}

/* 
 * load_program
 * Description:
 * - Duplicates $m[$r[B]], and replaces $m[0] with the duplicate. Sets the 
 *   program counter to $r[C]
 * Parameters:
 * - The load program instruction (Um_instruction instruction)
 * - Pointer to an initialized universal machine struct (UM um)
 * Effects:
 * - Frees segment 0 and stores a duplicate of $m[$r[B]] at segment 0. Sets 
 *   the program counter to $r[C]
 * Returns:
 * - None
 */
void load_program(Um_instruction inst, Um_instruction *registers, vector 
                  segmented_memory, uint32_vector segment_lengths, uint32_t* 
                  program_counter) 
{
    uint32_t bv = registers[get_reg_i(inst, 'b')];
    
    if (bv >= (uint32_t)vector_length(segmented_memory) ||
        (Um_instruction *)vector_get(segmented_memory, bv) == NULL) {
        fprintf(stderr, "Trying to load unmapped segment\n");
        exit(EXIT_FAILURE); /* Failure mode */
    }
    
    if (bv != 0) {
        /* Frees segment 0 */
        free((Um_instruction *)vector_get(segmented_memory, 0));
        
        /* Duplicates segment at $r[B] */
        uint32_t length_of_seq = uint32_vector_get(segment_lengths, bv);
        
        Um_instruction *new_segment = malloc(length_of_seq * 4);
        assert(new_segment != NULL);
        
        for (unsigned i = 0; i < length_of_seq; i++) {
            new_segment[i] = 
                      ((Um_instruction *)vector_get(segmented_memory, bv))[i];
        }
        
        // uint32_t *length_p = malloc(sizeof(length_of_seq));
        // assert(length_p != NULL);
        
        /* Stores duplicate at $m[0] */
        vector_put(segmented_memory, 0, new_segment);
        uint32_vector_put(segment_lengths, 0, length_of_seq);
    }
    
    *program_counter = registers[get_reg_i(inst, 'c')];
}

/************************** Helper Functions *********************************/

/* 
 * get_reg_i
 * Description:
 * - Gets the register index stored in the instruction passed in
 * Parameters:
 * - The instruction (Um_instruction instruction)
 * - The character value associated with the bitpacked register index 
 *   (char character - valid values are 'a', 'b', or 'c')
 * Effects:
 * - None
 * Returns:
 * - The index stored in the encoded bits associated with register indeces 
 *   (uint32_t - value from 0-7)
 */
uint32_t get_reg_i(Um_instruction instruction, char character) {
    
    if (character == 'c') {
        return bp_get_u(instruction, 3, 0);
    } else if (character == 'b') {
        return bp_get_u(instruction, 3, 3);
    } else if (character == 'a') {
        return bp_get_u(instruction, 3, 6);
    } 
    return 0;
}


uint64_t bp_get_u(uint64_t word, unsigned width, unsigned lsb) 
{
    assert(width <= 64);
    unsigned hi = lsb + width; /* one beyond the most significant bit */
    assert(hi <= 64);
    
    /* different type of right shift */
    uint64_t shl_return;
    unsigned bits = 64 - hi;
    assert(bits <= 64);
    if (bits == 64)
        shl_return =  0;
    else
        shl_return = word << bits;
    
    uint64_t shr_return;
    bits = 64 - width;
    assert(bits <= 64);
        if (bits == 64)
            shr_return = 0;
        else
            shr_return = shl_return >> bits;
    
    return shr_return; 
}


/* 

DYNAMIC INT ARRAY.C


*/

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
    int new_capacity = 2 * v->capacity;
    uint32_t* new_array = (uint32_t*) malloc(sizeof(int)*new_capacity);
    if (new_array == NULL) {
        fprintf(stderr, "Not enough memory!");
        abort();
    }
    int vsize = v->size;
    for(int i = 0; i < vsize; i++) {
        new_array[i] = v->array[i];
    }
    free(v->array);
    v->array = new_array;
    v->capacity = new_capacity;
}

static
void uint32_vector_half_capacity(uint32_vector v)
{
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
    int vsize = min(v->size, new_capacity);
    for(int i = 0; i < vsize; i++) {
        new_array[i] = v->array[i];
    }
    free(v->array);
    v->array = new_array;
    v->capacity = new_capacity;
    v->size = vsize;
}

void uint32_vector_addhi(uint32_vector v, uint32_t value) 
{
    if (v->size >= v->capacity) {
        uint32_vector_double_capacity(v);
    }
    v->array[v->size++] = value;
}

uint32_t uint32_vector_get(uint32_vector v, int i) 
{
    if (i >= v->size) {
        fprintf(stderr, "Out of index!");
        abort();
    }
    return v->array[i];
}
    
void uint32_vector_put(uint32_vector v, int i, uint32_t value) 
{
    if (i >= v->size) {
        fprintf(stderr, "Out of index!");
        abort();
    }
    v->array[i] = value;
}

void uint32_vector_add_at(uint32_vector v, int i, uint32_t value) 
{
    if (i >= v->size) {
        fprintf(stderr, "Out of index!");
        abort();
    }
    if (v->size >= v->capacity) {
        uint32_vector_double_capacity(v);
    }
    int vsize = v->size;
    for(int j = vsize; j > i; j--) {
        v->array[j] = v->array[j-1];
    }
    v->array[i] = value;
    v->size++;
}

uint32_t uint32_vector_remove_at(uint32_vector v, int i) 
{
    if (i >= v->size) {
        fprintf(stderr, "Out of index!");
        abort();
    }
    int ret = v->array[i];
    int vsize = v->size;
    
    for(int j = i + 1; j < vsize; j++) {
        v->array[j-1] = v->array[j];
    }
    v->size--;
    if (4 * vsize < v->capacity) {
        uint32_vector_half_capacity(v);
    }
    return ret;
}

int uint32_vector_is_empty(uint32_vector v) 
{
    return v->size == 0;
}

int uint32_vector_length(uint32_vector v) 
{
    return v->size;
}

void uint32_vector_clear(uint32_vector v) 
{
    v->size = 0;
    int vcapacity = v->capacity;
    while (vcapacity > INITIAL_CAPACITY) {
        uint32_vector_half_capacity(v);
    }
}

/*

DYNAMIC ARRAY . C



*/

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
    int new_capacity = 2 * v->capacity;
    value_type* new_array = (value_type*) malloc(sizeof(value_type)*new_capacity);
    if (new_array == NULL) {
        fprintf(stderr, "Not enough memory!");
        abort();
    }
    int vsize = v->size;
    for(int i = 0; i < vsize; i++) {
        new_array[i] = v->array[i];
    }
    free(v->array);
    v->array = new_array;
    v->capacity = new_capacity;
}

static
void vector_half_capacity(vector v)
{
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
    int vsize = min(v->size, new_capacity);
    for(int i = 0; i < vsize; i++) {
        new_array[i] = v->array[i];
    }
    free(v->array);
    v->array = new_array;
    v->capacity = new_capacity;
    v->size = vsize;
}

void vector_addhi(vector v, value_type value) 
{
    if (v->size >= v->capacity) {
        vector_double_capacity(v);
    }
    v->array[v->size++] = value;
}

value_type vector_get(vector v, int i) 
{
    if (i >= v->size) {
        fprintf(stderr, "Out of index!");
        abort();
    }
    return v->array[i];
}
    
void vector_put(vector v, int i, value_type value) 
{
    if (i >= v->size) {
        fprintf(stderr, "Out of index!");
        abort();
    }
    v->array[i] = value;
}

void vector_add_at(vector v, int i, value_type value) 
{
    if (i >= v->size) {
        fprintf(stderr, "Out of index!");
        abort();
    }
    if (v->size >= v->capacity) {
        vector_double_capacity(v);
    }
    int vsize = v->size;
    for(int j = vsize; j>i; j--) {
        v->array[j] = v->array[j-1];
    }
    v->array[i] = value;
    v->size++;
}

value_type vector_remove_at(vector v, int i) 
{
    if (i >= v->size) {
        fprintf(stderr, "Out of index!");
        abort();
    }
    value_type ret = v->array[i];
    int vsize = v->size;
    for(int j = i + 1; j < vsize; j++) {
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
    return v->size == 0;
}

int vector_length(vector v) 
{
    return v->size;
}

void vector_clear(vector v) 
{
    v->size = 0;
    int vcapacity = v->capacity;
    while (vcapacity > INITIAL_CAPACITY) {
        vector_half_capacity(v);
    }
}




