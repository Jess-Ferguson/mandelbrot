//mandelbrot.c - generates a .PPM (Portable Pixmap format, P6) file of the mandelbrot set with shading, has been optimised to use parallel processing
#include <stdio.h>
#include <complex.h>
#include <math.h>
#include <stdlib.h>
#include <omp.h>
#define MAX_TESTS 1000

typedef struct
{
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
	int xpx, ypx;
	double a, b;
	double complex num;
	FILE *file;
	dimensions dim;
	
	dim.height = atoi(argv[2]);
	
	if((file = fopen(file_name, "w")) != NULL)
	{
		dim = dim_gen(dim.height);
		fprintf(file, "P6 %d %d 255\n", dim.width, dim.height); //Magic number, image dimensions and max RGB value
		colour rgb[dim.width];
	
		#pragma omp parallel //Since the algorithm is 'perfectly parallel', the load can be easily distributed across all CPU cores 
		for(ypx = 0; ypx < dim.height; ypx++)
		{
			for(xpx = 0; xpx < dim.width; xpx++)
			{
				a = -2.0 + xpx * 2.5 / dim.width;
				b = 1.0 - ypx * 2.0 / dim.height;
				num = a + b * I;
				rgb[xpx] = mandelbrot_test(num);
			}
			fwrite(rgb, sizeof(colour), dim.width, file);
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
	double abs = c * conj(c);

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
	int brightness;
	
	if(iterations == MAX_TESTS)
	{
		rgb.red 	= 0;
		rgb.green 	= 0;
		rgb.blue 	= 0;
	}
    else
    {
		brightness 	= 256.0 * log2(iterations) / log2(MAX_TESTS - 1);
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
	return dim;
}

int error(const char *message)
{
	fprintf(stderr, "%s\n", message);
	return 1;
}
