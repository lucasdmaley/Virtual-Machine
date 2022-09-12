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
    int num_instructions = filesize >> 2;
    
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
    register uint64_t op_code = 0;
    char nothing;
    uint32_t av, bv, cv, num_instructions;
    uint64_t twopow32 = 4294967296;
    while (op_code != 7) {
        // fprintf(stderr, "in while\n");
        Um_instruction *while_segment_zero = um->segmented_memory->array[0];
        Um_instruction curr_instruction = while_segment_zero[um->program_counter]; 
        //op_code = bp_get_u(curr_instruction, 4, 28);
        op_code = (uint64_t)curr_instruction << 32;
        op_code = op_code >> 60;
        
        switch(op_code) {
            case 0: 
                (void) nothing;
                if (um->registers[get_reg_i(curr_instruction, 'c')] != 0) {
                    um->registers[get_reg_i(curr_instruction, 'a')] = um->registers[get_reg_i(curr_instruction, 'b')];
                }
                break;
            case 1: //segment load
                /* Store value of registers b and c */
                (void) nothing;

                bv = um->registers[get_reg_i(curr_instruction, 'b')];
                cv = um->registers[get_reg_i(curr_instruction, 'c')];
                
                if (bv >= (uint32_t)vector_length(um->segmented_memory) ||
                    (Um_instruction *)vector_get(um->segmented_memory, bv) == 0) {
                    fprintf(stderr, "Trying to load unmapped segment\n");
                    exit(EXIT_FAILURE); /* Failure mode */
                }
                
                num_instructions = uint32_vector_get(um->segment_lengths, bv);

                if (cv > num_instructions) {
                    fprintf(stderr, "Trying to access instruction out of bounds ");
                    fprintf(stderr, "of mapped segment\n");
                    exit(EXIT_FAILURE); /* Failure mode */
                }
                
                Um_instruction inst_at_rbrc = 
                ((Um_instruction *)vector_get(um->segmented_memory, bv))[cv];
                
                um->registers[get_reg_i(curr_instruction, 'a')] = inst_at_rbrc;
                break;
            case 2: //segment store
                (void) nothing;
                /* Store value of registers a, b, and c */
                av = um->registers[get_reg_i(curr_instruction, 'a')];
                bv = um->registers[get_reg_i(curr_instruction, 'b')];
                cv = um->registers[get_reg_i(curr_instruction, 'c')];
                
                if (av >= (uint32_t)vector_length(um->segmented_memory) ||
                    (Um_instruction *)vector_get(um->segmented_memory, av) == NULL) {
                    fprintf(stderr, "Trying to store in unmapped segment\n");
                    exit(EXIT_FAILURE); /* Failure mode */
                }
                
                num_instructions = uint32_vector_get(um->segment_lengths, av);
                
                if (bv > num_instructions) {
                    fprintf(stderr, "Trying to access instruction out of bounds ");
                    fprintf(stderr, "of mapped segment\n");
                    exit(EXIT_FAILURE); /* Failure mode */
                }

                ((Um_instruction *)vector_get(um->segmented_memory, av))[bv] = cv;
                break;
            case 3: //add
                (void) nothing;
                um->registers[get_reg_i(curr_instruction, 'a')] = (um->registers[get_reg_i(curr_instruction, 'b')] + um->registers[get_reg_i(curr_instruction, 'c')]) % twopow32;
                break;
            case 4: //multiply
                (void) nothing;
                um->registers[get_reg_i(curr_instruction, 'a')] = (um->registers[get_reg_i(curr_instruction,'b')] * um->registers[get_reg_i(curr_instruction, 'c')]) % twopow32;
                break;
            case 5: //divide
                (void) nothing;
                if (um->registers[get_reg_i(curr_instruction, 'c')] == 0) {
                    fprintf(stderr, "Cannot divide by zero.\n");
                    exit(EXIT_FAILURE); /* Failure mode */
                }
                um->registers[get_reg_i(curr_instruction, 'a')] = (um->registers[get_reg_i(curr_instruction, 'b')] / um->registers[get_reg_i(curr_instruction, 'c')]);
                break;
            case 6: //nand
                (void) nothing;
                um->registers[get_reg_i(curr_instruction, 'a')] = ~(um->registers[get_reg_i(curr_instruction, 'b')] & um->registers[get_reg_i(curr_instruction, 'c')]);
                break;
            case 7: //halt
                (void) nothing;
                int segmented_mem_len = vector_length(um->segmented_memory);
                for (int i = 0; i < segmented_mem_len; i++) {
                    free(vector_get(um->segmented_memory, i));
                }
                vector_free(um->segmented_memory);
                uint32_vector_free(um->segment_lengths);
                uint32_vector_free(um->unmappedID);
                break;
            case 8: //activate, map
                (void) nothing;
                /* Declares and initializes a new segment on the heap */
                int num_words = um->registers[get_reg_i(curr_instruction, 'c')];

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
                    um->registers[get_reg_i(curr_instruction, 'b')] = reusableID;
                } else {
                    /* Maps segment with new ID */
                    vector_addhi(um->segmented_memory, new_segment);
                    uint32_vector_addhi(um->segment_lengths, num_words);
                    um->registers[get_reg_i(curr_instruction, 'b')] = vector_length(um->segmented_memory) - 1;
                }
                break;
            case 9: //inactivate, unmap
                (void) nothing;
                uint32_t unmappedID = um->registers[get_reg_i(curr_instruction, 'c')];
            
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
                uint32_vector_addhi(um->unmappedID, unmappedID);
                break;
            case 10: //output
                (void) nothing;
                uint32_t c = um->registers[get_reg_i(curr_instruction, 'c')];
                if (c > 255) {
                /* Failure mode: unchecked runtime error */
                    fprintf(stderr, "Register c not within bounds\n");
                    exit(EXIT_FAILURE);
                    }
                putc(c, stdout);
                break;
            case 11: //input
                (void) nothing;
                int inputval = fgetc(stdin);
            
                /* Contract Violation */
                if (inputval < -1 || inputval > 255) { /* EOF == -1 */
                    fprintf(stderr, "Contract Violation: Must input value between 0 and");
                    fprintf(stderr, " 255\n");
                    exit(EXIT_FAILURE);
                }
            
                if (inputval != EOF) {
                    um->registers[get_reg_i(curr_instruction, 'c')] = inputval;
                } else {
                    um->registers[get_reg_i(curr_instruction, 'c')] = ~0;
                }
                break;
            case 12: //load program
                (void) nothing;
                bv = um->registers[get_reg_i(curr_instruction, 'b')];
            
                if (bv >= (uint32_t)vector_length(um->segmented_memory) ||
                    (Um_instruction *)vector_get(um->segmented_memory, bv) == NULL) {
                        fprintf(stderr, "Trying to load unmapped segment\n");
                        exit(EXIT_FAILURE); /* Failure mode */
                    }
            
                if (bv != 0) {
                /* Frees segment 0 */
                free((Um_instruction *)vector_get(um->segmented_memory, 0));
                
                /* Duplicates segment at $r[B] */
                uint32_t length_of_seq = uint32_vector_get(um->segment_lengths, bv);
                
                Um_instruction *new_segment = malloc(length_of_seq * 4);
                assert(new_segment != NULL);
                    
                Um_instruction *old_segment = (Um_instruction *)vector_get(um->segmented_memory, bv);
                for (unsigned i = 0; i < length_of_seq; i++) {
                    new_segment[i] = old_segment[i];
                }
                
                    /* Stores duplicate at $m[0] */
                    vector_put(um->segmented_memory, 0, new_segment);
                    uint32_vector_put(um->segment_lengths, 0, length_of_seq);
                }
            
                um->program_counter = um->registers[get_reg_i(curr_instruction, 'c')];
                um->program_counter--;
                break;
            case 13: //load value
                (void) nothing;
                uint32_t a = bp_get_u(curr_instruction, 3, 25);                
                um->registers[a] = bp_get_u(curr_instruction, 25, 0);
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
    uint64_t result;
    if (character == 'c') {
        result = (uint64_t)instruction << 61;
        return result >> 61;
    } else if (character == 'b') {
        result = (uint64_t)instruction << 58;
        return result >> 61;
    } else {
        result = (uint64_t)instruction << 55;
        return result >> 61;
    } 
}


uint64_t bp_get_u(uint64_t word, unsigned width, unsigned lsb) 
{
    uint64_t result = lsb + width;
    result = 64 - result;
    result = word << result;
    result = result >> (64 - width);

    return result;
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

void uint32_vector_addhi(uint32_vector v, uint32_t value) 
{
    if (v->size >= v->capacity) {
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
        if (v->capacity > INITIAL_CAPACITY) {
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

void vector_addhi(vector v, value_type value) 
{
    if (v->size >= v->capacity) {
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
        if (v->capacity > INITIAL_CAPACITY) {
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
}




