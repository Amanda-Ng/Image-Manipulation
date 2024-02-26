#include "hw2.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h> 

// Structure to store RGB values of a pixel
typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} Pixel;

// Structure to store image data
typedef struct {
    int width;
    int height;
    Pixel **pixels;  // 2D array to store pixel data
} Image;

// Function to check if a file exists
int fileExists(const char *filename) {
    FILE *file;
    if ((file = fopen(filename, "r"))) {
        fclose(file);
        return 1;
    }
    return 0;
}

int args_length(char *args){
    int count=1;
    for (int i=0; args[i]; i++){
        if(args[i] == ',' && args[i+1]!='\0')
        // count += (args[i] == ',');
        count += 1;
    }
    return count;
}

// Function to load PPM image from file
Image *load_ppm(const char *filename) {
    FILE *file = fopen(filename, "r");

    int width, height, max_val;
    fscanf(file, "%d %d %d", &width, &height, &max_val);

    // Allocate memory for image
    Image *image = (Image *)malloc(sizeof(Image));
    image->width = width;
    image->height = height;
    image->pixels = (Pixel **)malloc(height * sizeof(Pixel *));
    for (int i = 0; i < height; i++) {
        image->pixels[i] = (Pixel *)malloc(width * sizeof(Pixel));
    }

    // Read pixel data
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            fscanf(file, "%hhu %hhu %hhu", &image->pixels[i][j].red, &image->pixels[i][j].green, &image->pixels[i][j].blue);
        }
    }

    fclose(file);
    return image;
}
// Function to load SBU image from file
Image *load_sbu(const char *filename) {
    FILE *file = fopen(filename, "r");    

    int width, height, entries;
    fscanf(file, "%d %d %d", &width, &height, &entries);
    unsigned char colors[entries][3];

    // Allocate memory for image
    Image *image = (Image *)malloc(sizeof(Image));
    image->width = width;
    image->height = height;
    image->pixels = (Pixel **)malloc(height * sizeof(Pixel *));
    for (int i = 0; i < height; i++) {
        image->pixels[i] = (Pixel *)malloc(width * sizeof(Pixel));
    }

    // Read color table
    for(int i=0; i<entries; i++){
        fscanf(file, "%hhu %hhu %hhu", &colors[i][0], &colors[i][1], &colors[i][2]);
    }

    // Read pixel data
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int index=0;
            fscanf(file, "%d", &index);
            image->pixels[i][j].red=colors[index][0];
            image->pixels[i][j].green=colors[index][1];
            image->pixels[i][j].blue=colors[index][2];
        }
    }

    fclose(file);
    return image;
}
// Function to save PPM image to file
void save_ppm(const char *filename, Image *image) {
    FILE *file = fopen(filename, "w");

    // Write header
    fprintf(file, "P3\n%d %d\n255\n", image->width, image->height);

    // Write pixel data
    for (int i = 0; i < image->height; i++) {
        for (int j = 0; j < image->width; j++) {
            fprintf(file, "%hhu %hhu %hhu ", image->pixels[i][j].red, image->pixels[i][j].green, image->pixels[i][j].blue);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

// Function to save SBU image to file
void save_sbu(const char *filename, Image *image) {
    FILE *file = fopen(filename, "w");

    // Write header
    fprintf(file, "SBU\n%d %d\n", image->width, image->height);

    // Write color table
    // fprintf(file, "%d", sizeof(entries)/sizeof(entries[0]));
    // for(int l=0;l<sizeof(entries)/sizeof(entries[0]); l++){
    //     fprintf(file, " %u", entries[l][0]);
    //     fprintf(file, " %u", entries[l][1]);
    //     fprintf(file, " %u", entries[l][2]);
    // }
    // fprintf(file, "\n");
    fprintf(file, "%d", 255); // Maximum value for a color component
    for (int i = 0; i < image->height; i++) {
        for (int j = 0; j < image->width; j++) {
            // int index=-1;
            // for(int k=0;k<sizeof(entries)/sizeof(entries[0]);k++){
            //     if(image->pixels[i][j].red==entries[k][0] && image->pixels[i][j].green==entries[k][1] && image->pixels[i][j].blue==entries[k][2]){
            //         fprintf(file, " %d", k);
            //     }
            // }
        }
    }
    fprintf(file, "\n");

    fclose(file);
}

// Function to copy a region of pixels from the source image
Image *copy_region(Image *source, int start_row, int start_col, int width, int height) {

    // Allocate memory for the copied region
    Image *copied_region = (Image *)malloc(sizeof(Image));

    copied_region->width = width;
    copied_region->height = height;
    copied_region->pixels = (Pixel **)malloc(height * sizeof(Pixel *));

    for (int i = 0; i < height; i++) {
        copied_region->pixels[i] = (Pixel *)malloc(width * sizeof(Pixel));
    }

    // Copy pixel data from source image to copied region
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if(start_row + i <= source->height && start_col + j <= source->width){
                copied_region->pixels[i][j] = source->pixels[start_row + i][start_col + j];
            }
        }
    }

    return copied_region;
}

// Function to paste a copied region into the target image
void paste_region(Image *target, Image *copied_region, int dest_row, int dest_col) {

    // Paste the copied region into the target image
    for (int i = 0; i < copied_region->height; i++) {
        for (int j = 0; j < copied_region->width; j++) {
            if(dest_row + i <= target->height && dest_col + j <= target->width){
                target->pixels[dest_row + i][dest_col + j] = copied_region->pixels[i][j];
            }
        }
    }
}

int main(int argc, char **argv) {
    // (void)argc;
    // (void)argv;

    // return 1000;

    // Define variables to store command-line arguments
    char *input_file = NULL;
    char *output_file = NULL;
    char *c_args = NULL;
    char *p_args = NULL;
    char *r_args = NULL;
    char *r_arguments=NULL;
    // char *path_to_font = NULL;
    int i_argument_count = 0;
    int o_argument_count = 0;
    int c_argument_count = 0;
    int p_argument_count = 0;
    int r_argument_count = 0;

    // Process command-line arguments using getopt
    int opt;
    while ((opt = getopt(argc, argv, ":i:o:c:p:r:")) != -1) {
        switch (opt) {
            case 'i':
                i_argument_count++;
                if(optarg[0]=='-'){
                    fprintf(stderr, "Missing argument\n");
                    return MISSING_ARGUMENT;
                }else{
                    input_file = optarg;
                }
                break;
            case 'o':
                o_argument_count++;
                if(optarg[0]=='-'){
                    fprintf(stderr, "Missing argument\n");
                    return MISSING_ARGUMENT;
                }else{
                    output_file = optarg;
                }
                break;
            case 'c':
                c_argument_count++;
                if(optarg[0]=='-'){
                    fprintf(stderr, "Missing argument\n");
                    return MISSING_ARGUMENT;
                }
                c_args=optarg;
                // c_args = strtok(optarg, ",");
                break;
            case 'p':
                p_argument_count++;
                if(optarg[0]=='-'){
                    fprintf(stderr, "Missing argument\n");
                    return MISSING_ARGUMENT;
                }
                p_args=optarg;
                // p_args = strtok(optarg, ",");
                break;
            case 'r':
                r_argument_count++;
                if(optarg[0]=='-'){
                    fprintf(stderr, "Missing argument\n");
                    return MISSING_ARGUMENT;
                }else{
                    r_args=optarg;
                    r_arguments = strtok(optarg, ",");
                    // path_to_font = (r_args + 1);
                }
                break;

            case '?':
                    fprintf(stderr, "Unrecognized argument\n");
                    return UNRECOGNIZED_ARGUMENT;
            default:
                    fprintf(stderr, "Missing argument\n");
                    return MISSING_ARGUMENT;
        }
    }

    // Check for missing required arguments
    if (input_file == NULL || output_file == NULL) {
        fprintf(stderr, "Missing argument\n");
        return MISSING_ARGUMENT;
    }

    // Check for duplicate arguments
    if (i_argument_count > 1 || o_argument_count > 1 || c_argument_count > 1 || p_argument_count > 1 || r_argument_count > 1) {
        fprintf(stderr, "Duplicate argument\n");
        return DUPLICATE_ARGUMENT;
    }

    // Check if input file exists
    FILE *test_output = fopen(input_file, "r");
    if (!fileExists(input_file) || test_output==NULL) {
        fprintf(stderr, "Input file missing\n");
        return INPUT_FILE_MISSING;
    }
    fclose(test_output);

    // Check if output file is writable
    FILE *output_test = fopen(output_file, "w");
    if (output_test == NULL) {
        fprintf(stderr, "Output file unwritable\n");
        return OUTPUT_FILE_UNWRITABLE;
    }
    fclose(output_test);

    if(p_argument_count >= 1 && c_args==NULL){
        fprintf(stderr, "C Argument Missing\n");
        return C_ARGUMENT_MISSING;
    }

    if(args_length(c_args)<4 || args_length(c_args)>4){
        fprintf(stderr, "C Argument Invalid\n");
        return C_ARGUMENT_INVALID;
    }
    if(args_length(p_args)<2 || args_length(p_args)>2){
        fprintf(stderr, "P Argument Invalid\n");
        return P_ARGUMENT_INVALID;
    }
    FILE *font_test = fopen((r_arguments+1), "r");
    if(args_length(r_args)<5 || args_length(r_args)>5){
        fprintf(stderr, "R Argument Invalid\n");
        return R_ARGUMENT_INVALID;
    }
    fclose(font_test);
    
    // If all checks pass, return 0 indicating success
    return 0;

    // Image *image=NULL;
    // if(strstr(input_file, ".ppm")!=NULL){
    //     image=load_ppm(input_file);
    // }else if(strstr(input_file, ".sbu")!=NULL){
    //     image=load_sbu(input_file);
    // }

    // Image *copied_region = copy_region(image, (int)*c_args, (int)*(c_args+1), *(c_args+2), *(c_args+3));
    // paste_region(image, copied_region, (int)*p_args, (int)*(p_args+1));

    // if(strstr(output_file, ".ppm")!=NULL){
    //     save_ppm(output_file, image);
    // }else if(strstr(output_file, ".sbu")!=NULL){
    //     save_sbu(output_file, image);
    // }
    // //remove later
    // if (image) {
    //     printf("Image loaded successfully. Dimensions: %d x %d\n", image->width, image->height);
    // }
}
