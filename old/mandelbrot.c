//mandelbrot.c - generates a .PPM (Portable Pixmap format, P6) file of the mandelbrot set with shading
#include <stdio.h>
#include <complex.h>
#include <math.h>
#include <stdlib.h>
#define MAX_TESTS 1000

typedef struct
{
	double hinc;
	double winc;
	unsigned int height;
	unsigned int width;
} dimensions;

typedef struct
{
	unsigned char red;
	unsigned char green;
	unsigned char blue;
} colour;

int error(const char *message);
void time_est(int height);
colour mandelbrot_test(double complex c);
colour rgb_gen(int iterations);
dimensions dim_gen(int height);

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		return error("Correct usage: program_name image_name image_height");
	}
	
	char *file_name = argv[1];
	FILE* file;
	double complex num;
	colour rgb;
	dimensions dim;
	
	dim.height = atoi(argv[2]);
	file = fopen(file_name, "w");
	
	if(dim.height < 1000) //values under ~1000px cause scaling issues with the file
	{
		return error("Image cannot be less than 1000 px in height");
	}
	
	if(NULL != (file = fopen(file_name, "w")))
	{
		dim = dim_gen(dim.height);
		time_est(dim.height);
		fprintf(file, "P6 %d %d 255\n", dim.width + 1, dim.height); //Magic number, image dimensions and max RGB value

		{
			for (double j = -1.0; j <= 1.0; j += dim.hinc)
			{
				for (double i = -2.0; i <= 0.5; i += dim.winc)
				{
					num = i + j * I;
					rgb = mandelbrot_test(num);
					fwrite(&rgb, sizeof(char), sizeof(colour), file);
				}
			}
		}
		
		fclose(file);
	}
	else
	{
		return error("Unable to access file!");
	}

	return 0;
}

colour mandelbrot_test(double complex c)
{
	double complex x = 0;
	double abs = creal(c) * creal(c) + cimag(c) * cimag(c);

	if(abs * (8.0 * abs - 3.0) < 3.0/32.0 - creal(c)) //Quick test to see if we can bail out early
	{
		return rgb_gen(MAX_TESTS);
	} 
	
	for(int i = 1; i < MAX_TESTS; i++)
	{
		x *= x;
		x += c;

		if(cabs(x) > 2)
		{
			return rgb_gen(i);
		}
	}

	return rgb_gen(MAX_TESTS);
}

colour rgb_gen(int iterations)
{
	colour rgb;	
	
	if(iterations == MAX_TESTS)
	{
		rgb.red 	= 0;
		rgb.green 	= 0;
		rgb.blue 	= 0;
	}
    else
    {
        int brightness = 256.0 * log2(iterations) / log2(MAX_TESTS - 1);
        rgb.red 	= brightness;
		rgb.green 	= brightness;
		rgb.blue 	= 255;
    }
	return rgb;
}

dimensions dim_gen(int height)
{
	dimensions dim;
	dim.height 	= height;
	dim.width 	= 5.0 * height / 2.0;
	dim.hinc 	= 1.0 / (0.5 * dim.height);
	dim.winc 	= dim.hinc / 2.0;
	return dim;
}

void time_est(int height)
{
	printf("Estimated time: %.1fs\n", 0.0000059402 * height * height);
}

int error(const char *message)
{
	fprintf(stderr, "%s\n", message);
	return 1;
}
