/*
 * umlab.c
 *
 * Functions to generate UM unit tests. Once complete, this module
 * should be augmented and then linked against umlabwrite.c to produce
 * a unit test writing program.
 *  
 * A unit test is a stream of UM instructions, represented as a Hanson
 * Seq_T of 32-bit words adhering to the UM's instruction format.  
 * 
 * Any additional functions and unit tests written for the lab go
 * here. 
 *  
 */


#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <seq.h>
#include <bitpack.h>


typedef uint32_t Um_instruction;
typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
} Um_opcode;


/* Functions that return the two instruction types */

Um_instruction three_register(Um_opcode op, int ra, int rb, int rc) {
    Um_instruction result = 0;
    
    uint64_t op_u = (uint64_t)op;
    uint64_t ra_u = (uint64_t)ra;
    uint64_t rb_u = (uint64_t)rb;
    uint64_t rc_u = (uint64_t)rc;
    
    result = Bitpack_newu(result, 4, 28, op_u);
    result = Bitpack_newu(result, 3, 0, rc_u);
    result = Bitpack_newu(result, 3, 3, rb_u);
    result = Bitpack_newu(result, 3, 6, ra_u);
    
    return result;
}

Um_instruction loadval(unsigned ra, unsigned val) {
    

    Um_instruction result = 0;
    
    uint64_t ra_u = (uint64_t)ra;
    uint64_t val_u = (uint64_t)val;
    
    result = Bitpack_newu(result, 4, 28, LV);
    result = Bitpack_newu(result, 3, 25, ra_u);
    result = Bitpack_newu(result, 25, 0, val_u);
    
    return result;
}


/* Wrapper functions for each of the instructions */

static inline Um_instruction halt(void) 
{
        return three_register(HALT, 0, 0, 0);
}

typedef enum Um_register { r0 = 0, r1, r2, r3, r4, r5, r6, r7 } Um_register;

static inline Um_instruction add(Um_register a, Um_register b, Um_register c) 
{
        return three_register(ADD, a, b, c);
}

static inline Um_instruction mult(Um_register a, Um_register b, Um_register c) 
{
        return three_register(MUL, a, b, c);
}

static inline Um_instruction div(Um_register a, Um_register b, Um_register c) 
{
        return three_register(DIV, a, b, c);
}

static inline Um_instruction nand(Um_register a, Um_register b, Um_register c) 
{
        return three_register(NAND, a, b, c);
}

static inline Um_instruction output(Um_register c) 
{    
    return three_register(OUT, 0, 0, c);
}

static inline Um_instruction input(Um_register c)
{
    return three_register(IN, 0, 0, c);
}

static inline Um_instruction map(Um_register b, Um_register c)
{
    return three_register(ACTIVATE, 0, b, c);
}

static inline Um_instruction cmov(Um_register a, Um_register b, Um_register c)
{
    return three_register(CMOV, a, b, c);
}

static inline Um_instruction unmap(Um_register c)
{
    return three_register(INACTIVATE, 0, 0, c);
}

static inline Um_instruction sload(Um_register a, Um_register b, Um_register c)
{
    return three_register(SLOAD, a, b, c);
}

static inline Um_instruction sstore(Um_register a, Um_register b, 
                                    Um_register c)
{
    return three_register(SSTORE, a, b, c);
}

static inline Um_instruction loadp(Um_register b, Um_register c)
{
    return three_register(LOADP, 0, b, c);
}


/* Functions for working with streams */

static inline void append(Seq_T stream, Um_instruction inst)
{
        assert(sizeof(inst) <= sizeof(uintptr_t));
        Seq_addhi(stream, (void *)(uintptr_t)inst);
}

const uint32_t Um_word_width = 32;

void Um_write_sequence(FILE *output, Seq_T stream)
{
        assert(output != NULL && stream != NULL);
        int stream_length = Seq_length(stream);
        for (int i = 0; i < stream_length; i++) { /* for each instruction */
                Um_instruction inst = (uintptr_t)Seq_remlo(stream); 
                for (int lsb = Um_word_width - 8; lsb >= 0; lsb -= 8) {
                        fputc(Bitpack_getu(inst, 8, lsb), output);
                } /* outputs one instruction in big endian order  */
        }
      
}


/* Unit tests for the UM: See README for description of each test */

void build_halt_test(Seq_T stream)
{
        append(stream, halt());
}

void build_verbose_halt_test(Seq_T stream)
{
        append(stream, halt());
        append(stream, loadval(r1, 'B'));
        append(stream, output(r1));
        append(stream, loadval(r1, 'a'));
        append(stream, output(r1));
        append(stream, loadval(r1, 'd'));
        append(stream, output(r1));
        append(stream, loadval(r1, '!'));
        append(stream, output(r1));
        append(stream, loadval(r1, '\n'));
        append(stream, output(r1));
}

void build_add_test(Seq_T stream) 
{
    
    append(stream, add(r1, r2, r3));
    append(stream, halt());
}

void build_print_test(Seq_T stream)
{
    append(stream, loadval(r1, 48));
    append(stream, loadval(r2, 6));
    append(stream, add(r3, r1, r2));
    append(stream, output(r3));
    append(stream, halt());
}

void build_add_complete_test(Seq_T stream)
{
    /* Attempting the addition of different values */
    append(stream, loadval(r1, 7));
    append(stream, loadval(r2, 90));
    append(stream, add(r3, r1, r2));
    append(stream, output(r3));
    
    append(stream, loadval(r1, 0));
    append(stream, loadval(r2, 0));
    append(stream, add(r3, r1, r2));
    append(stream, output(r3));
    
    append(stream, loadval(r1, 1 << 24));
    append(stream, loadval(r2, 1 << 24));
    append(stream, add(r3, r1, r2));
    append(stream, halt());
}

void build_mult_complete_test(Seq_T stream)
{
    /* Attempting the multiplication of different values */
    append(stream, loadval(r1, 7));
    append(stream, loadval(r2, 4));
    append(stream, mult(r3, r1, r2));
    append(stream, output(r3));

    append(stream, loadval(r1, 0));
    append(stream, loadval(r2, 0));
    append(stream, mult(r3, r1, r2));
    append(stream, output(r3));

    append(stream, loadval(r1, 1 << 24));
    append(stream, loadval(r2, 1 << 24));
    append(stream, mult(r3, r1, r2));
    append(stream, halt());
}

void build_div_complete_test(Seq_T stream)
{
    /* Attempting the division of different values */
    append(stream, loadval(r1, 12));
    append(stream, loadval(r2, 4));
    append(stream, div(r3, r1, r2));
    append(stream, output(r3));

    append(stream, loadval(r1, 0));
    append(stream, loadval(r2, 0));
    append(stream, div(r3, r1, r2));
    append(stream, output(r3));

    append(stream, loadval(r1, 1 << 24));
    append(stream, loadval(r2, 1 << 24));
    append(stream, div(r3, r1, r2));
    append(stream, halt());
}

void build_nand_complete_test(Seq_T stream)
{
    append(stream, loadval(r4, 49));
    
    append(stream, loadval(r1, 0));
    append(stream, loadval(r2, 0));
    append(stream, nand(r3, r1, r2));
    append(stream, add(r5, r3, r4));
    append(stream, output(r5));
    
    append(stream, loadval(r1, 0));
    append(stream, loadval(r2, 5718));
    append(stream, nand(r3, r1, r2));
    append(stream, add(r5, r3, r4));
    append(stream, output(r5));
    
    append(stream, loadval(r1, 1));
    append(stream, loadval(r2, 0));
    append(stream, nand(r3, r1, r2));
    append(stream, add(r5, r3, r4));
    append(stream, output(r5));
    
    append(stream, loadval(r2, 1 << 24));
    append(stream, loadval(r6, 256));
    append(stream, mult(r7, r2, r6));
    append(stream, nand(r3, r7, r7));
    append(stream, add(r5, r3, r4));
    append(stream, output(r5));
    append(stream, halt());
}

 void build_input_complete_test(Seq_T stream)
 {
     append(stream, input(r1));
     append(stream, output(r1));
     append(stream, halt());
 }
 
 void build_map_complete_test(Seq_T stream) 
 {
     append(stream, loadval(r3, 48));
     append(stream, loadval(r2, 10));
     append(stream, map(r1, r2));
     append(stream, add(r1, r1, r3));
     append(stream, output(r1)); /* 1 */
     
     append(stream, map(r1, r2));
     append(stream, add(r1, r1, r3));
     append(stream, output(r1)); /* 2 */
     
     append(stream, loadval(r2, 1 << 24));
     append(stream, loadval(r6, 256));
     append(stream, mult(r7, r2, r6));
     append(stream, map(r1, r2));
     append(stream, add(r1, r1, r3));
     append(stream, output(r1)); /* 3 */
     append(stream, halt());
 }
 
 void build_c_move_complete_test(Seq_T stream) 
 {
     append(stream, loadval(r7, 48));
     
     append(stream, loadval(r2, 0)); /* C */
     append(stream, loadval(r3, 4)); /* B */
     append(stream, loadval(r4, 3)); /* A */
     
     append(stream, cmov(r4, r3, r2));
     append(stream, add(r4, r4, r7));
     append(stream, output(r4)); /* 3 */
     
     append(stream, loadval(r4, 3));
     append(stream, loadval(r2, 1));
     append(stream, cmov(r4, r3, r2));
     append(stream, add(r4, r4, r7));
     append(stream, output(r4)); /* 4 */
     append(stream, halt());
 }

void build_unmap_seg0_test(Seq_T stream)
{
    append(stream, loadval(r1, 0)); /* C */
    append(stream, unmap(r1)); /* expects Failure Mode */
    
    append(stream, halt());
}

void build_unmap_unmappedseg_test(Seq_T stream) 
{
    append(stream, loadval(r7, 48));
    append(stream, loadval(r1, 1));
    append(stream, unmap(r1));
    append(stream, halt());
}

void build_map_unmap_test(Seq_T stream)
{
    /* first mapping */
    append(stream, loadval(r7, 48));
    append(stream, loadval(r2, 10));
    append(stream, map(r1, r2));
    append(stream, add(r1, r1, r7));
    append(stream, output(r1)); /* 1 */
    
    /* unmapping */
    append(stream, loadval(r1, 1));
    append(stream, unmap(r1));
    
    /* second mapping - expects reuse first segment ID */
    append(stream, map(r1, r2));
    append(stream, add(r1, r1, r7));
    append(stream, output(r1)); /* 1 */
    
    append(stream, halt());
}

void build_seg_store_load_test(Seq_T stream) 
{
    append(stream, loadval(r7, 48));
    append(stream, loadval(r1, 3));
    append(stream, loadval(r2, 4));
    append(stream, loadval(r3, 5));
    append(stream, loadval(r4, 1));
    
    append(stream, map(r6, r3));
    append(stream, sstore(r4, r1, r2)); /* store 4 in segment 1, word 3 */
    append(stream, sload(r5, r4, r1)); /* takes value 4 from seg1, word 3 and 
                                          stores in r5 */
    
    append(stream, add(r5, r5, r7)); /* r5 becomes character 4 */
    append(stream, output(r5)); /* 4 */
    
    append(stream, halt());
}

void build_seg0_store_load_test(Seq_T stream)
{
    append(stream, loadval(r1, 0)); /* segment 0 */
    append(stream, loadval(r2, 9)); /* word 8 */
    append(stream, sload(r3, r1, r2));
    
    append(stream, loadval(r5, 1));
    append(stream, map(r4, r5));
    
    append(stream, sstore(r5, r1, r3)); /* puts the 8th instruction in seg 0 
                                           into first instruction of seg 1 */
    append(stream, loadp(r5, r1));
    
    append(stream, loadval(r7, 97)); /* will print a if loadp fails */
    append(stream, output(r7));
    append(stream, halt());
}

void build_load_unmapped_program_test(Seq_T stream)
{
    append(stream, loadval(r1, 3));
    append(stream, loadval(r2, 10));
    append(stream, unmap(r1)); /* unmap segment 1 */
    append(stream, loadp(r1, r2)); /* tries to load an unmapped segment, CRE */
    
    append(stream, halt());
}

void build_load_mapped_program_test(Seq_T stream)
{
    append(stream, loadval(r1, 3));
    append(stream, loadval(r2, 10));
    append(stream, map(r3, r2)); /* map a segment */
    append(stream, halt());
}

void build_load_seg0_program_test(Seq_T stream)
{
    append(stream, loadval(r4, 0));
    append(stream, loadval(r5, 3));
    append(stream, loadp(r4, r5));
    append(stream, halt());
}

void build_load_initialized_segment_test(Seq_T stream)
{
    append(stream, loadval(r1, 7));
    append(stream, loadval(r2, 1 << 14));
    append(stream, mult(r1, r2, r1)); /* 7 << 14 */
    append(stream, mult(r1, r2, r1)); /* 7 << 28 (the halt instruction) */
    
    append(stream, loadval(r3, 1));
    append(stream, map(r4, r3)); /* map a segment of length 1, 
                                    r4 holds bit pattern */
    
    append(stream, loadval(r5, 0));
    append(stream, sstore(r3, r5, r1)); /* put halt into segment 1 at word 0 */
    
    append(stream, loadp(r3, r5)); /* puts segment 1 into segment 0, resets 
    program counter */
    /* after this instruction, segment 0 should contain halt() */
}

void build_branch_test(Seq_T stream)
{
    append(stream, loadval(r2, 0));
    append(stream, loadval(r3, 8));
    append(stream, loadp(r2, r3)); 
    
    append(stream, halt());
    append(stream, halt());
    append(stream, halt()); /* block to go around */
    append(stream, halt());
    append(stream, halt());
    
    append(stream, loadval(r1, 97));
    append(stream, output(r1));
    append(stream, halt());
}

void build_extensive_map_unmap_test(Seq_T stream)
{
    append(stream, loadval(r1, 200));
    append(stream, map(r3, r1));
    append(stream, unmap(r3));
    append(stream, map(r3, r1));
    append(stream, unmap(r3));
    append(stream, map(r3, r1));
    append(stream, unmap(r3));
    append(stream, map(r3, r1));
    append(stream, unmap(r3));
    append(stream, map(r3, r1));
    append(stream, unmap(r3));
    append(stream, map(r3, r1));
    append(stream, unmap(r3));
    append(stream, halt());
}

void build_loop_test(Seq_T stream)
{
    append(stream, loadval(r0, 7)); /* [0] */
    append(stream, loadval(r2, 1)); /* [1] */
    append(stream, loadval(r3, 2)); /* [2] */
    append(stream, loadval(r4, 1)); /* [3] */
    append(stream, loadval(r7, 0)); /* [4] r7 == 0 */
    append(stream, nand(r6, r7, r7)); /* [5] r6 == ~0 */
    append(stream, loadval(r7, 0)); /* [6] r7 == 0 */
    
    append(stream, loadval(r1, 13)); /* [7] */
    append(stream, mult(r2, r2, r3)); /* [8] */
    append(stream, add(r2, r2, r4)); /* [9] r2 << 1 */
    
    append(stream, nand(r5, r2, r6)); /* [10] when r2 and r6 are all 1s, 
                                         r5 == 0 */
    append(stream, cmov(r1, r0, r5)); /* [11] if r5 is not 0, r1 == 7. 
                                              if r5 is 0, r1 = 13 */
    append(stream, loadp(r7, r1)); /* [12] goes to segment 0, instruction r1 */
    
    append(stream, halt()); /* [13] */
}


