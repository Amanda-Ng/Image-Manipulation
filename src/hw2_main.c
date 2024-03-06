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
typedef struct
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} Pixel;

// Structure to store image data
typedef struct
{
    int width;
    int height;
    Pixel **pixels; // 2D array to store pixel data
} Image;

// Function to check if a file exists
int fileExists(const char *filename)
{
    FILE *file;
    if ((file = fopen(filename, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}

int args_length(char *args)
{
    int count = 1;
    for (int i = 0; args[i]; i++)
    {
        if (args[i] == ',' && args[i + 1] != '\0')
            // count += (args[i] == ',');
            count += 1;
    }
    return count;
}

// Function to load PPM image from file
Image *load_ppm(const char *filename)
{
    FILE *file = fopen(filename, "r");

    int width, height, max_val;
    fscanf(file, "%*s %d %d %d", &width, &height, &max_val);

    // Allocate memory for image
    Image *image = (Image *)malloc(sizeof(Image));
    image->width = width;
    image->height = height;
    image->pixels = (Pixel **)malloc(height * sizeof(Pixel *));
    for (int i = 0; i < height; i++)
    {
        image->pixels[i] = (Pixel *)malloc(width * sizeof(Pixel));
    }

    // Read pixel data
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            fscanf(file, "%hhu %hhu %hhu", &image->pixels[i][j].red, &image->pixels[i][j].green, &image->pixels[i][j].blue);
        }
    }

    fclose(file);
    return image;
}
// Function to load SBU image from file
Image *load_sbu(const char *filename)
{
    FILE *file = fopen(filename, "r");

    int width, height, entries;
    fscanf(file, "%*s %d %d %d", &width, &height, &entries);
    unsigned char colors[entries][3];

    // Allocate memory for image
    Image *image = (Image *)malloc(sizeof(Image));
    image->width = width;
    image->height = height;
    image->pixels = (Pixel **)malloc(height * sizeof(Pixel *));
    for (int i = 0; i < height; i++)
    {
        image->pixels[i] = (Pixel *)malloc(width * sizeof(Pixel));
    }

    // Read color table
    for (int i = 0; i < entries; i++)
    {
        fscanf(file, "%hhu %hhu %hhu", &colors[i][0], &colors[i][1], &colors[i][2]);
    }

    // for(int i=0; i<entries; i++){
    //     printf("red: %d", colors[i][0]);
    //     printf("green: %d", colors[i][1]);
    //     printf("blue: %d", colors[i][2]);
    // }

    // Read pixel data with run-length encoding
    int pixelIndex = 0;
    int x = 0, y = 0;
    while (y < height)
    {
        int count, index;
        char next_char;
        int matched_items = fscanf(file, "%c%d", &next_char, &index);
        if (matched_items == 2)
        {
            if (next_char != ' ' && next_char != '\n')
            {
                // printf("%c%d\n", next_char, index);
                // RLE encoding
                count = index;
                fscanf(file, "%c%d", &next_char, &index);
            }
            else
            {
                // Single pixel
                count = 1;
            }

            for (int i = 0; i < count; i++)
            {
                image->pixels[y][x].red = colors[index][0];
                image->pixels[y][x].green = colors[index][1];
                image->pixels[y][x].blue = colors[index][2];

                pixelIndex++;
                x++;
                if (x == width)
                {
                    x = 0;
                    y++;
                }
            }
        }
    }

    fclose(file);
    return image;
}
// Function to save PPM image to file
void save_ppm(const char *filename, Image *image)
{
    FILE *file = fopen(filename, "w");

    // Write header
    fprintf(file, "P3\n%d %d\n255\n", image->width, image->height);

    // Write pixel data
    for (int i = 0; i < image->height; i++)
    {
        for (int j = 0; j < image->width; j++)
        {
            fprintf(file, "%hhu %hhu %hhu ", image->pixels[i][j].red, image->pixels[i][j].green, image->pixels[i][j].blue);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

// Function to save SBU image to file
void save_sbu(const char *filename, Image *image)
{
    FILE *file = fopen(filename, "w");
    // Write SBU header
    fprintf(file, "SBU\n");
    fprintf(file, "%d %d\n", image->width, image->height);
    // Color table generation
    int color_table_size = 0;
    Pixel color_table[image->width * image->height];
    for (int i = 0; i < image->height; i++)
    {
        for (int j = 0; j < image->width; j++)
        {
            // Check if the current pixel color exists in the color table
            int k;
            for (k = 0; k < color_table_size; k++)
            {
                if (color_table[k].red == image->pixels[i][j].red &&
                    color_table[k].green == image->pixels[i][j].green &&
                    color_table[k].blue == image->pixels[i][j].blue)
                {
                    break;
                }
            }
            // If the color is not found in the color table, add it
            if (k == color_table_size)
            {
                // if(k<10){
                // printf("red: %d", image->pixels[i][j].red);
                // printf("green: %d", image->pixels[i][j].green);
                // printf("blue: %d", image->pixels[i][j].blue);}
                color_table[color_table_size] = image->pixels[i][j];
                color_table_size++;
            }
        }
    }
    // Write color table size and entries
    fprintf(file, "%d ", color_table_size);
    for (int i = 0; i < color_table_size; i++)
    {
        fprintf(file, "%hhu %hhu %hhu ", color_table[i].red, color_table[i].green, color_table[i].blue);
    }
    fprintf(file, "\n");
    // Pixel data
    int run_length = 1;
    Pixel prev_pixel = image->pixels[0][0];
    for (int i = 0; i < image->height; i++)
    {
        for (int j = 0; j < image->width; j++)
        {
            Pixel current_pixel = image->pixels[i][j];
            Pixel next_pixel;
            // initialize next_pixel
            if (j == (image->width - 1) && i == (image->height - 1))
            {
                // last pixel
                if (current_pixel.red == prev_pixel.red &&
                    current_pixel.green == prev_pixel.green &&
                    current_pixel.blue == prev_pixel.blue)
                {
                    // last pixel in run length
                    run_length++;
                    fprintf(file, "*%d ", run_length);
                }
                for (int k = 0; k < color_table_size; k++)
                {
                    if (current_pixel.red == color_table[k].red &&
                        current_pixel.green == color_table[k].green &&
                        current_pixel.blue == color_table[k].blue)
                    {
                        fprintf(file, "%d ", k);
                        break;
                    }
                }
            }
            else if (j == (image->width - 1))
            {
                next_pixel = image->pixels[i + 1][0];
            }
            else
            {
                next_pixel = image->pixels[i][j + 1];
            }

            if ((current_pixel.red == prev_pixel.red &&
                 current_pixel.green == prev_pixel.green &&
                 current_pixel.blue == prev_pixel.blue) &&
                !((i == 0) && (j == 0)))
            {
                // printf("run length increased");
                // Same color as previous pixel, increase run length
                run_length++;
                if ((current_pixel.red != next_pixel.red ||
                     current_pixel.green != next_pixel.green ||
                     current_pixel.blue != next_pixel.blue) &&
                    run_length > 1)
                {
                    // end of run length
                    //  printf("end of run length");
                    fprintf(file, "*%d ", run_length);
                    run_length = 1;
                    for (int k = 0; k < color_table_size; k++)
                    {
                        if (current_pixel.red == color_table[k].red &&
                            current_pixel.green == color_table[k].green &&
                            current_pixel.blue == color_table[k].blue)
                        {
                            fprintf(file, "%d ", k);
                            break;
                        }
                    }
                }
            }
            else if (current_pixel.red == next_pixel.red &&
                     current_pixel.green == next_pixel.green &&
                     current_pixel.blue == next_pixel.blue)
            {
                // start of run length
                run_length = 1;
            }
            else
            {
                // different from previous and next pixel
                //  Different color, write run-length encoded pixel data
                //  if ((current_pixel.red != next_pixel.red ||
                //  current_pixel.green != next_pixel.green ||
                //  current_pixel.blue != next_pixel.blue) && run_length > 1)
                //  {
                //      fprintf(file, "*%d ", run_length);
                //      run_length = 1;
                //  }
                //  Write index of the current pixel in the color table
                for (int k = 0; k < color_table_size; k++)
                {
                    if (current_pixel.red == color_table[k].red &&
                        current_pixel.green == color_table[k].green &&
                        current_pixel.blue == color_table[k].blue)
                    {
                        fprintf(file, "%d ", k);
                        break;
                    }
                }
                // prev_pixel = current_pixel;
            }
            prev_pixel = current_pixel;
        }
    }
    // Write any remaining run-length encoded pixels
    // if (run_length > 1)
    // {
    //     fprintf(file, "*%d ", run_length);
    //     Pixel current_pixel = image->pixels[(image->height) - 1][(image->width) - 1];
    //     for (int k = 0; k < color_table_size; k++)
    //     {
    //         if (current_pixel.red == color_table[k].red &&
    //             current_pixel.green == color_table[k].green &&
    //             current_pixel.blue == color_table[k].blue)
    //         {
    //             fprintf(file, "%d ", k);
    //             break;
    //         }
    //     }
    // }
    fclose(file);
}

// Function to copy a region of pixels from the source image
Image *copy_region(Image *source, int start_row, int start_col, int width, int height)
{

    // Allocate memory for the copied region
    Image *copied_region = (Image *)malloc(sizeof(Image));

    int actual_width = (start_col + width <= source->width) ? width : source->width - start_col;
    int actual_height = (start_row + height <= source->height) ? height : source->height - start_row;

    copied_region->width = actual_width;
    copied_region->height = actual_height;
    copied_region->pixels = (Pixel **)malloc(actual_height * sizeof(Pixel *));

    for (int i = 0; i < actual_height; i++)
    {
        copied_region->pixels[i] = (Pixel *)malloc(actual_width * sizeof(Pixel));
    }

    // Copy pixel data from source image to copied region
    for (int i = 0; i < actual_height; i++)
    {
        for (int j = 0; j < actual_width; j++)
        {
            if (start_row + i < source->height && start_col + j < source->width)
            {
                copied_region->pixels[i][j] = source->pixels[start_row + i][start_col + j];
            }
        }
    }

    return copied_region;
}

// Function to paste a copied region into the target image
void paste_region(Image *target, Image *copied_region, int dest_row, int dest_col)
{

    // Paste the copied region into the target image
    for (int i = 0; i < copied_region->height; i++)
    {
        for (int j = 0; j < copied_region->width; j++)
        {
            if (dest_row + i < target->height && dest_col + j < target->width)
            {
                target->pixels[dest_row + i][dest_col + j] = copied_region->pixels[i][j];
            }
        }
    }
}

// // Load font from file
// char **load_font(const char *filename, int *font_height)
// {
//     FILE *file = fopen(filename, "r");

//     int lines_allocated = 10;
//     int max_line_len = 150;
//     char **lines = (char **)malloc(sizeof(char *) * lines_allocated);

//     int num_lines = 0;
//     while (fgets(lines[num_lines], max_line_len, file) != NULL)
//     {
//         num_lines++;
//     }

//     fclose(file);
//     *font_height = num_lines;
//     return lines;
// }

Image **parse_font(const char *filename){
    //return an array of images where each image is a letter in the font
    FILE* file = fopen(filename, "r");
    int font_height = 1;
    char c;
    int current_col;
    Image** letter_images = (Image**)malloc(26 * sizeof(Image*));

    //Account for extra column
    if(strcmp(filename,"fonts/font3.txt")){
        current_col = 1;
    }else{
        current_col = 0;
    }

    // Count the number of lines
    while ((c = fgetc(file)) != EOF) {
        if (c == '\n') {
            font_height++;
        }
    }
    
    // Reset the file pointer to the beginning of the file
    fseek(file, 0, SEEK_SET);

    //find width and allocate memory
    for(int i=0; i<26; i++){
        letter_images[i] = (Image *)malloc(sizeof(Image));
        letter_images[i]->pixels = (Pixel **)malloc(font_height * sizeof(Pixel *));
        int letter_width = 0;
        for(int j=current_col; j<current_col+10; j++){
            //find first empty space
            char ch;
            int spaces_found;
            fseek(file, j, SEEK_SET); // Move to position j in first row
            if ((ch = fgetc(file)) == ' '){
                spaces_found = 1;
            }else{
                continue;
            }

            //Check for empty spaces in the same column
            for(int k=1; k<font_height; k++){
                //skip to next line at position j
                while ((c = fgetc(file)) != EOF && c != '\n') {
                }
                fseek(file, j, SEEK_CUR);

                //increment number of spaces if empty space found
                if ((ch = fgetc(file)) == ' '){
                    spaces_found++;
                }
            }
            if(spaces_found == font_height){
                letter_width = j-current_col;
                for(int l=0; l<font_height; l++){
                    letter_images[i]->pixels[l] = (Pixel *)malloc(letter_width * sizeof(Pixel));
                }
                letter_images[i]->height = font_height;
                letter_images[i]->width = letter_width;
                current_col = j+1;
                if(strcmp(filename,"fonts/font2.txt") && i==17){
                    //Account for extra column
                    current_col++;
                }
                break;
            }
        }
    }

    //Reset current_col
    if(strcmp(filename,"fonts/font3.txt")){
        current_col = 1;
    }else{
        current_col = 0;
    }

    //copy letter to image
    for(int i=0; i<26; i++){

        fseek(file, current_col, SEEK_SET); // Move to current_col in first row
        char ch;

        for(int j=0; j<font_height; j++){

                //skip to next line
                if(j>0){
                    while ((ch = fgetc(file)) != EOF && ch != '\n') {
                    }
                    //skip to position current_col
                    fseek(file, current_col, SEEK_CUR);
                }

            for(int k=0; k<letter_images[i]->width; k++){

                if((ch = fgetc(file)) == '*'){
                    letter_images[i]->pixels[j][k].red = 255;
                    letter_images[i]->pixels[j][k].green = 255;
                    letter_images[i]->pixels[j][k].blue = 255;
                }else{
                    letter_images[i]->pixels[j][k].red = 0;
                    letter_images[i]->pixels[j][k].green = 0;
                    letter_images[i]->pixels[j][k].blue = 0;
                }
            }
        }
        current_col = (current_col + letter_images[i]->width + 1);
    }

    fclose(file);
    return letter_images;
}

void free_image(Image *image)
{
    for (int i = 0; i < image->height; i++)
    {
        free(image->pixels[i]);
    }
    free(image->pixels);
    free(image);
}

void print_message(Image *image, const char *message, const char *font_filename, int font_size, int row, int col){
    //Allocate memory
    Image **letter_images = parse_font(font_filename);
    Image **message_letter_images = (Image**)malloc((int)strlen(message) * sizeof(Image*));
    for(int i=0; i<(int)strlen(message); i++){
        message_letter_images[i] = (Image *)malloc(sizeof(Image));
        message_letter_images[i]->pixels = (Pixel **)malloc(letter_images[0]->height * font_size * sizeof(Pixel *));
        for(int j=0;j<(letter_images[0]->height*font_size);j++){
            message_letter_images[i]->pixels[j] = (Pixel *)malloc(letter_images[(toupper(message[i])-'A')]->width *font_size * sizeof(Pixel));
        }
    }

    //Create scaled images
    for(int m=0; m<(int)strlen(message);m++){
        if(message[m]==' '){
            continue;
        }
        for(int i=0;i<letter_images[(toupper(message[m])-'A')]->height;i++){
            for(int j=0; j<letter_images[(toupper(message[m])-'A')]->width; j++){
                Pixel original = letter_images[(toupper(message[m])-'A')]->pixels[i][j];
                for(int k=0;k<font_size;k++){
                    for(int l=0;l<font_size;l++){
                        if(original.red==255 && original.green==255 && original.blue==255){
                            message_letter_images[m]->pixels[i*font_size + k][j*font_size+l] = original;
                        }
                    }
                }
            }
        }
    }

    //Paste images
    int current_col = col;

    for(int k=0;k<(int)strlen(message);k++){
        if(message[k]==' '){
            current_col += 5;
            continue;
        }
        if (row + message_letter_images[k]->height < image->height && current_col + message_letter_images[k]->width < image->width){
            for (int i = 0; i < message_letter_images[k]->height; i++)
            {
                for (int j = 0; j < message_letter_images[k]->width; j++)
                {
                    image->pixels[row + i][current_col + j] = message_letter_images[k]->pixels[i][j];
                }
            }
            current_col += 1;
        }
        else{
            break;
        }
    }

    for(int k=0; k<26;k++){
        free_image(letter_images[k]);
    }
    free(letter_images);
    for(int l=0;l<(int)strlen(message);l++){
        free_image(message_letter_images[l]);
    }
    free(message_letter_images);
}

// // TODO: account for scaling (assumed 6), number of columns per char, extra column
// void print_message(Image *image, const char *message, const char *font_filename, int font_size, int row, int col)
// {
//     int font_height;
//     char **font = load_font(font_filename, &font_height);

//     int message_length = strlen(message);
//     int current_col = col;

//     int scaled_font_height = font_height * font_size;

//     for (int i = 0; i < message_length; i++)
//     {
//         char current_char = message[i];

//         if (current_char >= 'a' && current_char <= 'z')
//         {
//             current_char -= 32; // Convert to uppercase
//         }

//         if (current_char == ' ')
//         {
//             current_col += 5;
//             continue;
//         }

//         if (current_col + 6 * font_size > image->width || row + scaled_font_height > image->height)
//         {
//             break; // Message runs off the image
//         }

//         for (int j = 0; j < font_height; j++)
//         {
//             for (int k = 0; k < 6; k++)
//             {
//                 if (font[current_char - 'A'][j] == '*')
//                 {
//                     // White pixel
//                     image->pixels[row + j * font_size][current_col + k * font_size].red = 255;
//                     image->pixels[row + j * font_size][current_col + k * font_size].green = 255;
//                     image->pixels[row + j * font_size][current_col + k * font_size].blue = 255;
//                 }
//             }
//         }

//         current_col += ((6 * font_size) + 1); // Advance 6 columns and leave 1 column space
//     }
// }

// void free_image(Image *image)
// {
//     for (int i = 0; i < image->height; i++)
//     {
//         free(image->pixels[i]);
//     }
//     free(image->pixels);
//     free(image);
// }

int main(int argc, char **argv)
{
    // (void)argc;
    // (void)argv;

    // return 1000;

    char *input_file = NULL;
    char *output_file = NULL;
    char *c_args = NULL;
    char *p_args = NULL;
    char *r_args = NULL;
    // char *r_arguments = NULL;
    // char *path_to_font = NULL;
    int i_argument_count = 0;
    int o_argument_count = 0;
    int c_argument_count = 0;
    int p_argument_count = 0;
    int r_argument_count = 0;

    int opt;
    while ((opt = getopt(argc, argv, ":i:o:c:p:r:")) != -1)
    {
        switch (opt)
        {
        case 'i':
            i_argument_count++;
            if (optarg[0] == '-')
            {
                fprintf(stderr, "Missing argument\n");
                return MISSING_ARGUMENT;
            }
            else
            {
                input_file = optarg;
            }
            break;
        case 'o':
            o_argument_count++;
            if (optarg[0] == '-')
            {
                fprintf(stderr, "Missing argument\n");
                return MISSING_ARGUMENT;
            }
            else
            {
                output_file = optarg;
            }
            break;
        case 'c':
            c_argument_count++;
            if (optarg[0] == '-')
            {
                fprintf(stderr, "Missing argument\n");
                return MISSING_ARGUMENT;
            }
            c_args = optarg;
            // c_args = strtok(optarg, ",");
            break;
        case 'p':
            p_argument_count++;
            if (optarg[0] == '-')
            {
                fprintf(stderr, "Missing argument\n");
                return MISSING_ARGUMENT;
            }
            p_args = optarg;
            // p_args = strtok(optarg, ",");
            break;
        case 'r':
            r_argument_count++;
            if (optarg[0] == '-')
            {
                fprintf(stderr, "Missing argument\n");
                return MISSING_ARGUMENT;
            }
            else
            {
                r_args = optarg;
                // r_arguments = strtok(optarg, ",");
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
    if (input_file == NULL || output_file == NULL)
    {
        fprintf(stderr, "Missing argument\n");
        return MISSING_ARGUMENT;
    }

    if (i_argument_count > 1 || o_argument_count > 1 || c_argument_count > 1 || p_argument_count > 1 || r_argument_count > 1)
    {
        fprintf(stderr, "Duplicate argument\n");
        return DUPLICATE_ARGUMENT;
    }

    FILE *test_output = fopen(input_file, "r");
    if (test_output == NULL)
    {
        fprintf(stderr, "Input file missing\n");
        return INPUT_FILE_MISSING;
    }
    // fclose(test_output);

    FILE *output_test = fopen(output_file, "w");
    if (output_test == NULL)
    {
        fprintf(stderr, "Output file unwritable\n");
        return OUTPUT_FILE_UNWRITABLE;
    }
    // fclose(output_test);

    if (p_argument_count >= 1 && c_args == NULL)
    {
        fprintf(stderr, "C Argument Missing\n");
        return C_ARGUMENT_MISSING;
    }

    if ((c_argument_count == 1) && (args_length(c_args) < 4 || args_length(c_args) > 4))
    {
        fprintf(stderr, "C Argument Invalid\n");
        return C_ARGUMENT_INVALID;
    }
    if ((p_argument_count == 1) && (args_length(p_args) < 2 || args_length(p_args) > 2))
    {
        fprintf(stderr, "P Argument Invalid\n");
        return P_ARGUMENT_INVALID;
    }
    // (void)r_arguments;
    // (void)r_args;
    if (r_argument_count == 1)
    {
        // FILE *font_test = fopen(path_to_font, "r");
        if (args_length(r_args) < 5 || args_length(r_args) > 5)
        {
            fprintf(stderr, "R Argument Invalid\n");
            return R_ARGUMENT_INVALID;
        }
        // fclose(font_test);
    }

    Image *image = NULL;
    if (strstr(input_file, ".ppm") != NULL)
    {
        image = load_ppm(input_file);
    }
    else if (strstr(input_file, ".sbu") != NULL)
    {
        image = load_sbu(input_file);
    }

    if (c_args)
    {
        const char *token;
        int c_arguments[5];
        int i = 0;
        token = strtok(c_args, ",");
        while (token != NULL)
        {
            c_arguments[i] = atoi(token);
            i++;
            token = strtok(NULL, ",");
        }
        //           for(int j=0; j<5;j++){
        //       printf("%d\n", c_arguments[j]);
        //   }


        // const char *c_arguments = strtok(c_args, ",");
        Image *copied_region = copy_region(image, c_arguments[0], c_arguments[1], c_arguments[2], c_arguments[3]);
        // (void)c_arguments;
        // (void)copied_region;
        if (p_args)
        {
            const char *token1;
            int p_arguments[2];
            int k = 0;
            token1 = strtok(p_args, ",");
            while (token1 != NULL)
            {
                p_arguments[k] = atoi(token1);
                k++;
                token1 = strtok(NULL, ",");
            }
            paste_region(image, copied_region, p_arguments[0], p_arguments[1]);
            // for (int j = 0; j < 2; j++)
            // {
            //     printf("%d\n", p_arguments[k]);
            // }
            // printf("%d\n", p_arguments[1]);

            // (void)p_arguments;

            // const char *p_arguments = strtok(p_args, ",");
            // paste_region(image, copied_region, atoi(p_arguments + 2), atoi(p_arguments + 3));
        }
    }

    if(r_args){
        const char *token;
        const char *r_arguments[5];
        int i = 0;
        token = strtok(r_args, ",");
        while (token != NULL)
        {
            r_arguments[i] = token;
            i++;
            token = strtok(NULL, ",");
        }
        print_message(image, r_arguments[0], r_arguments[1], atoi(r_arguments[2]), atoi(r_arguments[3]), atoi(r_arguments[4]));

    }

    if (strstr(output_file, ".ppm") != NULL)
    {
        save_ppm(output_file, image);
    }
    else if (strstr(output_file, ".sbu") != NULL)
    {
        save_sbu(output_file, image);
    }
    // remove later
    if (image)
    {
        printf("Image loaded successfully. Dimensions: %d x %d\n", image->width, image->height);
    }
    // (void)r_arguments;
    (void)r_args;
    (void)p_args;
    (void)c_args;

    free_image(image);

    return 0;
}
