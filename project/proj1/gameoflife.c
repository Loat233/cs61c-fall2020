/************************************************************************
**
** NAME:        gameoflife.c
**
** DESCRIPTION: CS61C Fall 2020 Project 1
**
** AUTHOR:      Justin Yokota - Starter Code
**				YOUR NAME HERE
**
**
** DATE:        2020-08-23
**
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "imageloader.h"

//Determines what color the cell at the given row/col should be. This function allocates space for a new Color.
//Note that you will need to read the eight neighbors of the cell in question. The grid "wraps", so we treat the top row as adjacent to the bottom row
//and the left column as adjacent to the right column.
Color *evaluateOneCell(Image *image, int row, int col, uint32_t rule)
{
	uint32_t rows = image->rows;
	uint32_t cols = image->cols;
	Color* next_color = (Color*) malloc(sizeof(Color));
	next_color->R = 0;
	next_color->G = 0;
	next_color->B = 0;

	int relative_row[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
	int relative_col[8] = {-1, 0 ,1, -1, 1, -1, 0, 1};

	// each bit in R/G/B
	for (uint8_t bit = 0; bit < 8; bit++) {
		// alive neighbors of each bit
		uint8_t alive_neigh_R = 0;
		uint8_t alive_neigh_G = 0;
		uint8_t alive_neigh_B = 0;

		// 8 neighbors
		for (uint8_t i = 0; i < 8; i++) {
			int n_row = (row + relative_row[i] + rows) % rows;
			int n_col = (col + relative_col[i] + cols) % cols;
			Color neigh_colors = image->image[n_row][n_col];

			alive_neigh_R += neigh_colors.R >> bit & 1;
			alive_neigh_G += neigh_colors.G >> bit & 1;
			alive_neigh_B += neigh_colors.B >> bit & 1;
		}
		// evaluate current state bit
		uint8_t state_R = (image->image[row][col].R >> bit) & 1;
		uint8_t state_G = (image->image[row][col].G >> bit) & 1;
		uint8_t state_B = (image->image[row][col].B >> bit) & 1;

		// evaluate next state bit
		uint8_t next_state_R = (rule >> (state_R * 9 + alive_neigh_R)) & 1;
		uint8_t next_state_G = (rule >> (state_G * 9 + alive_neigh_G)) & 1;
		uint8_t next_state_B = (rule >> (state_B * 9 + alive_neigh_B)) & 1;

		// assignment
		next_color->R |= next_state_R << bit;
		next_color->G |= next_state_G << bit;
		next_color->B |= next_state_B << bit;
	}
	return next_color;
}

//The main body of Life; given an image and a rule, computes one iteration of the Game of Life.
//You should be able to copy most of this from steganography.c
Image *life(Image *image, uint32_t rule)
{
	Image *new_image = (Image*) malloc(sizeof(Image));
	new_image->image = (Color**) malloc(image->rows * sizeof(Color*));
	if (!new_image->image) {
		free(new_image);
		return NULL;
	}
	new_image->cols = image->cols;
	new_image->rows = image->rows;

	for (uint32_t i = 0; i < image->rows; i++) {
		new_image->image[i] = (Color*) malloc(image->cols * sizeof(Color));
		for (uint32_t j = 0; j < image->cols; j++) {
			Color *color = evaluateOneCell(image, i, j, rule);
			new_image->image[i][j] = *color;
			free(color);
		}
	}
	return new_image;
}

/*
Loads a .ppm from a file, computes the next iteration of the game of life, then prints to stdout the new image.

argc stores the number of arguments.
argv stores a list of arguments. Here is the expected input:
argv[0] will store the name of the program (this happens automatically).
argv[1] should contain a filename, containing a .ppm.
argv[2] should contain a hexadecimal number (such as 0x1808). Note that this will be a string.
You may find the function strtol useful for this conversion.
If the input is not correct, a malloc fails, or any other error occurs, you should exit with code -1.
Otherwise, you should return from main with code 0.
Make sure to free all memory before returning!

You may find it useful to copy the code from steganography.c, to start.
*/
int main(int argc, char **argv)
{
	if (argc != 3) {
		printf("usage: ./gameOfLife filename rule\n");
		printf("filename is an ASCII PPM file (type P3) with maximum value 255.\n");
		printf("rule is a hex number beginning with 0x; Life is 0x1808.\n");
		return -1;
	}

	char *filename = argv[1];
	char *rule_str = argv[2];

	char *end_ptr;
	uint32_t rule = strtol(rule_str, &end_ptr, 16);
	if (*end_ptr != '\0' || rule < 0x00000 || rule > 0x3FFFF) {
		printf("Invalid rule. Rule should be a hex number between 0x00000 and 0x3FFFF.\n");
		return -1;
	}

	Image *image = readData(filename);
	Image *new_image = life(image, rule);

	writeData(new_image);

	freeImage(new_image);
	freeImage(image);
}
