#include "hw2.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h> 

// Function to check if a file exists
int fileExists(const char *filename) {
    FILE *file;
    if ((file = fopen(filename, "r"))) {
        fclose(file);
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    // (void)argc;
    // (void)argv;

    // return 1000;

    // Define variables to store command-line arguments
    char *input_file = NULL;
    char *output_file = NULL;
    int c_argument_count = 0;
    int p_argument_count = 0;
    int r_argument_count = 0;

    // Process command-line arguments using getopt
    int opt;
    while ((opt = getopt(argc, argv, "i:o:c:p:r:")) != -1) {
        switch (opt) {
            case 'i':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            case 'c':
                c_argument_count++;
                break;
            case 'p':
                p_argument_count++;
                break;
            case 'r':
                r_argument_count++;
                break;
            default:
                // If an unrecognized argument is provided, print error and exit
                fprintf(stderr, "Unrecognized argument\n");
                return UNRECOGNIZED_ARGUMENT;
        }
    }

    // Check for missing required arguments
    if (input_file == NULL || output_file == NULL) {
        fprintf(stderr, "Missing argument\n");
        return MISSING_ARGUMENT;
    }

    // Check for duplicate arguments
    if (c_argument_count > 1 || p_argument_count > 1 || r_argument_count > 1) {
        fprintf(stderr, "Duplicate argument\n");
        return DUPLICATE_ARGUMENT;
    }

    // Check if input file exists
    if (!fileExists(input_file)) {
        fprintf(stderr, "Input file missing\n");
        return INPUT_FILE_MISSING;
    }

    // Check if output file is writable
    FILE *test_output = fopen(output_file, "w");
    if (test_output == NULL) {
        fprintf(stderr, "Output file unwritable\n");
        return OUTPUT_FILE_UNWRITABLE;
    }
    fclose(test_output);

    // Additional checks for -c, -p, -r arguments can be implemented here

    // If all checks pass, return 0 indicating success
    return 0;
}
