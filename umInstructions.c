/* umInstructions.c
 * HW06: um
 * Lucas Maley and Colby Cho
 * Implementation of the Universal Machine instruction set.
 */
 
 #include "umInstructions.h"
 
 typedef enum Um_opcode {
         CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
         NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
 } Um_opcode;


/********************* Private Function Declarations *************************/
/* UM Instructions */
static inline void conditional_move(Um_instruction inst, 
                                    Um_instruction *registers);
static inline void segmented_load(Um_instruction inst, UM um);
static inline void segmented_store(Um_instruction inst, UM um);

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
static inline void load_program(Um_instruction inst, UM um);
static inline void load_value(Um_instruction inst, Um_instruction *registers);

/* Helper Function */
uint32_t get_reg_i(Um_instruction instruction, char character);

/************************** Function Definitions *****************************/

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
    uint16_t op_code = 0;
    
    while (op_code != 7) {
        
        Um_instruction curr_instruction = 
        ((Um_instruction *)Seq_get(um->segmented_memory, 0))
                                                         [um->program_counter];
        
        op_code = Bitpack_getu(curr_instruction, 4, 28);
        
        switch(op_code) {
            case 0: 
                conditional_move(curr_instruction, um->registers); 
                break;
            case 1: 
                segmented_load(curr_instruction, um);
                break;
            case 2: 
                segmented_store(curr_instruction, um);
                break;
            case 3: 
                addition(curr_instruction, um->registers);
                break;
            case 4: 
                multiplication(curr_instruction, um->registers);
                break;
            case 5: 
                division(curr_instruction, um->registers);
                break;
            case 6: 
                bitwise_NAND(curr_instruction, um->registers);
                break;
            case 7: 
                halt(um);
                break;
            case 8: 
                map_segment(curr_instruction, um);
                break;
            case 9: 
                unmap_segment(curr_instruction, um);
                break;
            case 10: 
                output(curr_instruction, um->registers);
                break;
            case 11: 
                input(curr_instruction, um->registers);
                break;
            case 12:
                load_program(curr_instruction, um);
                um->program_counter--;
                break;
            case 13:
                load_value(curr_instruction, um->registers);
                break;
            default: 
                /* Failure mode: opcode out of range */
                fprintf(stderr, "Invalid opcode\n");
                exit(EXIT_FAILURE);
                break;
        }
        um->program_counter++;
    }
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
    uint32_t a = Bitpack_getu(instruction, 3, 25);
    registers[a] = Bitpack_getu(instruction, 25, 0);
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
    for (int i = 0; i < Seq_length(um->segmented_memory); i++) {
        if (((Um_instruction *)Seq_get(um->segmented_memory, i)) != NULL) { 
            free(((Um_instruction *)Seq_get(um->segmented_memory, i)));
            free(((uint32_t *)Seq_get(um->segment_lengths, i)));
        }
    }
    Seq_free(&um->segment_lengths);
    Seq_free(&um->segmented_memory);
    
    /* Frees IDs of unmapped segments */
    for (int i = 0; i < Seq_length(um->unmappedID); i++) {
        free(((uint32_t *)Seq_get(um->unmappedID, i)));
    }
    Seq_free(&um->unmappedID);
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
    
    uint32_t *num_words_p = malloc(sizeof(num_words)); /* potential reason for failure: malloc sizeof int*/
    *num_words_p = num_words;                           /* do we free sizof something else? */
    
    /* Check if any segments have been unmapped whose IDs can be reused */
    if (Seq_length(um->unmappedID) != 0) {
        /* Maps segment with ID of previously unmapped segment */
        uint32_t reusableID = 
                          *(uint32_t *)Seq_get(um->unmappedID, 
                                               Seq_length(um->unmappedID) - 1);
        free((uint32_t *)Seq_remhi(um->unmappedID));
        Seq_put(um->segmented_memory, reusableID, new_segment);
        Seq_put(um->segment_lengths, reusableID, num_words_p);
        um->registers[get_reg_i(inst, 'b')] = reusableID;
    } else {
        /* Maps segment with new ID */
        Seq_addhi(um->segmented_memory, new_segment);
        Seq_addhi(um->segment_lengths, num_words_p);
        um->registers[get_reg_i(inst, 'b')] = Seq_length(um->segmented_memory)
                                              - 1;
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
        unmappedID >= (uint32_t)Seq_length(um->segmented_memory)) {
        fprintf(stderr, "Can't unmap segment 0 or non-mapped segments\n");
        exit(EXIT_FAILURE); /* Failure mode */
    }
    
    /* Unmaps the segment */
    free(((Um_instruction *)Seq_get(um->segmented_memory, unmappedID)));
    Seq_put(um->segmented_memory, unmappedID, NULL);
    
    free(((uint32_t *)Seq_get(um->segment_lengths, unmappedID)));
    Seq_put(um->segment_lengths, unmappedID, NULL);
    
    /* Adds the ID of the unmapped segment to the associated Hanson Sequence */
    uint32_t *unmappedID_p = malloc(sizeof(unmappedID));
    assert(unmappedID_p != NULL);
    *unmappedID_p = unmappedID;
    
    Seq_addhi(um->unmappedID, unmappedID_p);
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
void segmented_load(Um_instruction inst, UM um)
{
    /* Store value of registers b and c */
    uint32_t bv = um->registers[get_reg_i(inst, 'b')];
    uint32_t cv = um->registers[get_reg_i(inst, 'c')];
    
    if (bv >= (uint32_t)Seq_length(um->segmented_memory) ||
        (Um_instruction *)Seq_get(um->segmented_memory, bv) == NULL) {
        fprintf(stderr, "Trying to load unmapped segment\n");
        exit(EXIT_FAILURE); /* Failure mode */
    }
    
    uint32_t num_instructions = *(uint32_t *)Seq_get(um->segment_lengths, bv);

    if (cv > num_instructions) {
        fprintf(stderr, "Trying to access instruction out of bounds ");
        fprintf(stderr, "of mapped segment\n");
        exit(EXIT_FAILURE); /* Failure mode */
    }
    
    Um_instruction inst_at_rbrc = 
    ((Um_instruction *)Seq_get(um->segmented_memory, bv))[cv];

    um->registers[get_reg_i(inst, 'a')] = inst_at_rbrc;
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
void segmented_store(Um_instruction inst, UM um)
{
    /* Store value of registers a, b, and c */
    uint32_t av = um->registers[get_reg_i(inst, 'a')];
    uint32_t bv = um->registers[get_reg_i(inst, 'b')];
    uint32_t cv = um->registers[get_reg_i(inst, 'c')];
    
    if (av >= (uint32_t)Seq_length(um->segmented_memory) ||
        (Um_instruction *)Seq_get(um->segmented_memory, av) == NULL) {
        fprintf(stderr, "Trying to store in unmapped segment\n");
        exit(EXIT_FAILURE); /* Failure mode */
    }
    
    uint32_t num_instructions = *(uint32_t *)Seq_get(um->segment_lengths, av);
    
    if (bv > num_instructions) {
        fprintf(stderr, "Trying to access instruction out of bounds ");
        fprintf(stderr, "of mapped segment\n");
        exit(EXIT_FAILURE); /* Failure mode */
    }

    ((Um_instruction *)Seq_get(um->segmented_memory, av))[bv] = cv;
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
void load_program(Um_instruction inst, UM um) 
{
    uint32_t bv = um->registers[get_reg_i(inst, 'b')];
    
    if (bv >= (uint32_t)Seq_length(um->segmented_memory) ||
        (Um_instruction *)Seq_get(um->segmented_memory, bv) == NULL) {
        fprintf(stderr, "Trying to load unmapped segment\n");
        exit(EXIT_FAILURE); /* Failure mode */
    }
    
    if (bv != 0) {
        /* Frees segment 0 */
        free((Um_instruction *)Seq_get(um->segmented_memory, 0));
        free((uint32_t *)Seq_get(um->segment_lengths, 0));
        
        /* Duplicates segment at $r[B] */
        uint32_t length_of_seq = *(uint32_t *)Seq_get(um->segment_lengths, bv);
        
        Um_instruction *new_segment = malloc(length_of_seq * 4);
        assert(new_segment != NULL);
        
        for (unsigned i = 0; i < length_of_seq; i++) {
            new_segment[i] = 
                      ((Um_instruction *)Seq_get(um->segmented_memory, bv))[i];
        }
        
        uint32_t *length_p = malloc(sizeof(length_of_seq));
        assert(length_p != NULL);
        *length_p = length_of_seq; //add this
        
        /* Stores duplicate at $m[0] */
        Seq_put(um->segmented_memory, 0, new_segment);
        Seq_put(um->segment_lengths, 0, length_p); /* potential cause of failure */
    }
    
    um->program_counter = um->registers[get_reg_i(inst, 'c')];
    
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
    
    uint32_t registerIndex = 0;
    if (character == 'c') {
        registerIndex = Bitpack_getu(instruction, 3, 0);
    } else if (character == 'b') {
        registerIndex = Bitpack_getu(instruction, 3, 3);
    } else if (character == 'a') {
        registerIndex = Bitpack_getu(instruction, 3, 6);
    } 
    return registerIndex;
}