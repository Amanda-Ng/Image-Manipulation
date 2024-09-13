# Image Manipulation Program

## Overview: 
a command-line program that reads an image from disk, performs various image manipulation operations, and writes the modified image back to disk.

## Supported Image Formats:
- Custom SBU image format (.sbu)
- Standard PPM image format (.ppm)

## Command-line Arguments:
- -i <input_file>: Path to the input image file (either .sbu or .ppm).
- -o <output_file>: Path to save the modified image (either .sbu or .ppm).
- -c <row,col,width,height>: Copy a rectangular region of the image.
- -p <row,col>: Paste the copied region to a new location in the image.
- -r <message,font_file,font_size,row,col>: Print a message in white text on the image using a specified font.

## Operations:
- Copy-Paste Operation: Copy a selected region of the image and paste it at a specified position.
- Text Printing: Print a message in white on the image using a specified font and size.

## Usage
- The program is executed by passing command-line arguments to specify the image file and the desired operations. 
- Example usage: ```./image_editor -i "./images/face.sbu" -o "./images/modified.ppm" -c 5,8,20,30 -p 19,50```
- This command loads face.sbu, copies a region from row 5, column 8 of size 20x30, and pastes it starting at row 19, column 50 in the output file modified.ppm.
