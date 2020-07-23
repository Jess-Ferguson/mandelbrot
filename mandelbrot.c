/* 
 * Filename:	mandelbrot.c
 * Author:	Jess Turner
 * Date:	9-AUG-2016
 * Licence:	GNU GPL V3
 *
 * Generates a .ppm (Portable Pixmap format, P6) file of the mandelbrot set with shading
 * 
 * Must be compiled with -lm and -fopenmp flags
 *
 * Options:
 *	-f [output file name] (required)
 *	-h [image height in px] (required)
 *	-t [max imaginary component]
 *	-b [min imaginary component]
 *	-v [max real component]
 *	-n [min real component]
 *
 * Return/exit codes:
 *	EXEC_SUCCESS	- Successful execution
 *	ARG_ERROR	- Argument error
 *	FILE_ERROR	- File access error
 *	SIZE_ERROR	- Image height error
 *	MEM_ERROR	- Memory allocation error
 */

#include <stdio.h>
#include <complex.h>
#include <math.h>
#include <stdlib.h>
#include <omp.h>
#include <ctype.h>
#include <unistd.h>
#include <png.h>
#include <limits.h>

#define MAX_TESTS 2000

#define EXEC_SUCCESS 0
#define ARG_ERROR -1
#define FILE_ERROR -2
#define SIZE_ERROR -3
#define MEM_ERROR -4

typedef struct {
	unsigned int height;
	unsigned int width;
	double ymax, ymin;
	double xmax, xmin;
	char *file_name;
} image_meta;

typedef struct {
	unsigned short red;
	unsigned short green;
	unsigned short blue;
} colour;

image_meta image_meta_gen(int argc, char *argv[]);	/* Generates the meta data for the image by parsing the input arguments */

static inline short mandelbrot_test(double complex c)
{
	double complex x = 0;
	double abs	 = c * conj(c);
	int i;

	if(abs * (8.0 * abs - 3.0) < 3.0 / 32.0 - creal(c))	/* Quick test to see if we can bail out early by checking if the number lies within the main cardioid */
		return MAX_TESTS;				/* Full disclosure, I have no idea how that equation works */
	
	for(i = 1; i < MAX_TESTS; i++) {
		x *= x;
		x += c;

		if(cabs(x) >= 2) {
			return i;
		}
	}

	return MAX_TESTS;
}

static inline colour rgb_gen(short iterations)
{
	colour rgb = { .red = USHORT_MAX, .green = USHORT_MAX, .blue = USHORT_MAX };
	
	if(iterations != MAX_TESTS) {
		short brightness	= USHORT_MAX * log2(iterations) / log2(MAX_TESTS - 1);
		rgb.red		= brightness;
		rgb.green	= brightness;
		rgb.blue	= USHORT_MAX;
	}
	
	return rgb;
}

int main(int argc, char *argv[])
{
	image_meta image;
	image = image_meta_gen(argc, argv);
	
	FILE * file;
	colour * rgb;
	double xdiffratio, ydiffratio;
	
	if(!(rgb = malloc(image.width * image.height * sizeof(colour)))) {
		fprintf(stderr, "Memory allocation error!\n");
		exit(MEM_ERROR);
	}
	
	if(!(file = fopen(image.file_name, "w"))) {
		fprintf(stderr, "File access error!\n");
		free(rgb);
		exit(FILE_ERROR);
	} else {
		printf("Generating image...\n");
		fprintf(file, "P6 %d %d 255\n", image.width, image.height);

		xdiffratio = (image.xmax - image.xmin) / image.width;
		ydiffratio = (image.ymax - image.ymin) / image.height;
		
		#pragma omp parallel 
		{
			#pragma omp for schedule(dynamic)
				for(unsigned int ypx = 0; ypx < image.height; ypx++) {
					for(unsigned int xpx = 0; xpx < image.width; xpx++) {
						register double complex num = (image.xmin + xpx * xdiffratio) + (image.ymax - ypx * ydiffratio) * I;
						rgb[ypx * image.width + xpx] = rgb_gen(mandelbrot_test(num));
					}
				}
		}
		
		fwrite(rgb, sizeof(colour), image.width * image.height, file);
		fclose(file);
	}

	exit(EXEC_SUCCESS);
}

image_meta image_meta_gen(int argc, char *argv[])
{
	int c;
	image_meta image;
	
	image.file_name	= NULL;
	opterr			= 0;
	image.xmax		= image.ymax = image.xmin = image.ymin = -1;
	image.height	= 0;
	image.width		= 0;

	while((c = getopt(argc, argv, "h:f:t:b:v:n:")) != -1) {
		switch(c) {
			case 'h':
				image.height = atoi(optarg);
				break;
			case 't':
				image.ymax = atof(optarg);
				break;
			case 'b':
				image.ymin = atof(optarg);
				break;
			case 'v':
				image.xmax = atof(optarg);
				break;
			case 'n':
				image.xmin = atof(optarg);
				break;
			case 'f':
				image.file_name = optarg;
				break;
			case '?':
				if(optopt == 'f' || optopt == 'h')
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				else if(isprint(optopt))
					fprintf(stderr, "Unknown option '-%c'.\n", optopt);
				else
					fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
					
				fprintf(stderr, "Usage: %s [-f file_name] [-h image_height]\n"
						"Optional: [-t y_max] [-b y_min] [-v x_min] [-n x_max]\n", argv[0]);
				exit(ARG_ERROR);
		}
	}
	
	if(argc < 4) {
		fprintf(stderr, "Error:\nToo few args!\n"
				"Usage: %s [-f file_name] [-h image_height]\n"
				"Optional: [-t y_max] [-b y_min] [-v x_min] [-n x_max]\n", argv[0]);
		exit(ARG_ERROR);
	}
	
	if(image.height < 30) {
		fprintf(stderr, "Error:\nHeight can't be less than 30!\n");
		exit(SIZE_ERROR);
	}
	
	if(image.xmax == image.xmin) {
		image.xmax = 0.8;
		image.xmin = -2.0;
		printf("Using default x values...\n");
	}
	
	if(image.ymax == image.ymin) {
		image.ymax = 1.2;
		image.ymin = -1.2;
		printf("Using default y values...\n");
	}
	
	if(image.xmin > image.xmax) {
		double temp	= image.xmin;
		image.xmin	= image.xmax;
		image.xmax	= temp;
	}
	
	if(image.ymin > image.ymax) {
		double temp	= image.ymin;
		image.ymin	= image.ymax;
		image.ymax	= temp;
	}
	
	image.width = image.height * (image.xmax - image.xmin) / (image.ymax - image.ymin);
	
	return image;
}
