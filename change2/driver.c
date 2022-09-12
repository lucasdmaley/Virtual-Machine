/* driver.c
 * HW06: um
 * Lucas Maley and Colby Cho
 * Contains main - functions as the driver of the Universal Machine
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "umInstructions.h"
#include "um.h"


/* 
 * checkCommandline
 * Description:
 * - Checks the command line for a valid program call
 * Parameters:
 * - Number of arguments on the command line (int argc)
 * - Array of arguments passed to command line (char** argv)
 * Effects:
 * - If an invalid call is made, exits the program.
 * Returns:
 * - None
 */
void checkCommandline(int argc, char** argv)
{
    if (argc != 2) {
        fprintf(stderr, "Innapropriate number of command line arguments\n");
        exit(EXIT_FAILURE);
    }

    char *ext = strrchr(argv[1], '.');
    if (!ext) { /* File extension does not exist */
        fprintf(stderr, "Innapropriate file extension\n");     
        exit(EXIT_FAILURE);
    }

    if ((strcmp(ext + 1, "um") != 0) && (strcmp(ext + 1, "umz") != 0)) {   
        fprintf(stderr, "Innapropriate file extension\n");     
        exit(EXIT_FAILURE);
    } 
}

int main(int argc, char** argv) 
{
    /* check file extension */
    checkCommandline(argc, argv);
    
    /* Create an instance of a UM*/
    UM um = initialize_UM();
    load_mem(argv[1], um);
    
    /* Executes the instructions given in the file given on the command line */
    execute_instr(um);
    
    /* Frees heap allocated memory associated with the Universal Machine */
    free(um);
}