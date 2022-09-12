#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "fmt.h"
#include "seq.h"

extern void Um_write_sequence(FILE *output, Seq_T instructions);

extern void build_halt_test(Seq_T instructions);
extern void build_verbose_halt_test(Seq_T instructions);
extern void build_add_test(Seq_T instructions);
extern void build_print_test(Seq_T instruction);
extern void build_add_complete_test(Seq_T stream);
extern void build_mult_complete_test(Seq_T stream);
extern void build_div_complete_test(Seq_T stream);
extern void build_nand_complete_test(Seq_T stream);
extern void build_input_complete_test(Seq_T stream);
extern void build_map_complete_test(Seq_T stream);
extern void build_c_move_complete_test(Seq_T stream);
extern void build_unmap_seg0_test(Seq_T stream);
extern void build_unmap_unmappedseg_test(Seq_T stream);
extern void build_map_unmap_test(Seq_T stream);
extern void build_seg_store_load_test(Seq_T stream);
extern void build_load_unmapped_program_test(Seq_T stream);
extern void build_load_mapped_program_test(Seq_T stream);
extern void build_load_seg0_program_test(Seq_T stream);
extern void build_load_initialized_segment_test(Seq_T stream);
extern void build_seg0_store_load_test(Seq_T stream);
extern void build_extensive_map_unmap_test(Seq_T stream);
extern void build_branch_test(Seq_T stream);
extern void build_loop_test(Seq_T stream);


/* The array `tests` contains all unit tests */

static struct test_info {
        const char *name;
        const char *test_input;          /* NULL means no input needed */
        const char *expected_output;
        /* writes instructions into sequence */
        void (*build_test)(Seq_T stream);
} tests[] = {
        { "halt",         NULL, "", build_halt_test },
        { "halt-verbose", NULL, "", build_verbose_halt_test },
        { "add",          NULL, "", build_add_test},
        { "print-six",        NULL, "6", build_print_test},
        { "print",        NULL, "6", build_print_test},
        { "add-complete", NULL, "a", build_add_complete_test},
        { "mult-complete", NULL, "", build_mult_complete_test},
        { "div-complete", NULL, "Cannot divide by zero.", 
           build_div_complete_test},
        { "nand-complete", NULL, "0000", build_nand_complete_test},
        { "input-complete", "&\n", "&", build_input_complete_test},
        { "map-complete", NULL, "123", build_map_complete_test},
        { "conditional_move-complete", NULL, "34", 
          build_c_move_complete_test },
        { "unmap-seg0", NULL, 
          "Can't unmap segment 0 or non-mapped segments", 
          build_unmap_seg0_test },
        { "unmap-unmappedseg", NULL, 
          "Can't unmap segment 0 or non-mapped segments",
          build_unmap_unmappedseg_test },
        { "map-unmap", NULL, "11", build_map_unmap_test},
        { "seg-store_load", NULL, "4", build_seg_store_load_test },
        { "load-unmapped_program", NULL, 
          "Can't unmap segment 0 or non-mapped segments", 
          build_load_unmapped_program_test },
        { "load-mapped_program", NULL, "", build_load_mapped_program_test },
        { "load-seg0", NULL, "", build_load_seg0_program_test },
        { "load-initialized_segment", NULL, "", 
          build_load_initialized_segment_test},
        { "branch", NULL, "a", build_branch_test },
        { "seg0-load", NULL, "", build_seg0_store_load_test },
        { "extensive-map_unmap", NULL, "", build_extensive_map_unmap_test },
        { "loop", NULL, "", build_loop_test}
};

  
#define NTESTS (sizeof(tests)/sizeof(tests[0]))

/*
 * open file 'path' for writing, then free the pathname;
 * if anything fails, checked runtime error
 */
static FILE *open_and_free_pathname(char *path);

/*
 * if contents is NULL or empty, remove the given 'path', 
 * otherwise write 'contents' into 'path'.  Either way, free 'path'.
 */
static void write_or_remove_file(char *path, const char *contents);

static void write_test_files(struct test_info *test);


int main (int argc, char *argv[])
{
        bool failed = false;
        if (argc == 1)
                for (unsigned i = 0; i < NTESTS; i++) {
                        printf("***** Writing test '%s'.\n", tests[i].name);
                        write_test_files(&tests[i]);
                }
        else
                for (int j = 1; j < argc; j++) {
                        bool tested = false;
                        for (unsigned i = 0; i < NTESTS; i++)
                                if (!strcmp(tests[i].name, argv[j])) {
                                        tested = true;
                                        write_test_files(&tests[i]);
                                }
                        if (!tested) {
                                failed = true;
                                fprintf(stderr,
                                        "***** No test named %s *****\n",
                                        argv[j]);
                        }
                }
        return failed; /* failed nonzero == exit nonzero == failure */
}


static void write_test_files(struct test_info *test)
{
        FILE *binary = open_and_free_pathname(Fmt_string("%s.um", test->name));
        Seq_T instructions = Seq_new(0);
        test->build_test(instructions);
        Um_write_sequence(binary, instructions);
        Seq_free(&instructions);
        fclose(binary);

        write_or_remove_file(Fmt_string("%s.0", test->name),
                             test->test_input);
        write_or_remove_file(Fmt_string("%s.1", test->name),
                             test->expected_output);
}


static void write_or_remove_file(char *path, const char *contents)
{
        if (contents == NULL || *contents == '\0') {
                remove(path);
        } else {
                FILE *input = fopen(path, "wb");
                assert(input != NULL);

                fputs(contents, input);
                fclose(input);
        }
        free(path);
}


static FILE *open_and_free_pathname(char *path)
{
        FILE *fp = fopen(path, "wb");
        assert(fp != NULL);

        free(path);
        return fp;
}
