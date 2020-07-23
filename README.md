# mandelbrot.c

Filename:	mandelbrot.c

Author:         Jess Turner

Date:           9/08/16

Licence:	GNU GPL V3


Generates a .ppm (Portable Pixmap format, P6) file of the mandelbrot set with shading
 
Must be compiled with -lm and -fopenmp flags

#Options:

-	-f [output file name] (required)
-	-h [image height in px] (required)
-	-t [max imaginary component]
-	-b [min imaginary component]
-	-v [max real component]
-	-n [min real component]


#Return/exit codes:

-	EXEC_SUCCESS	- Successful execution
-	ARG_ERROR	- Argument error
-	FILE_ERROR	- File access error
-	SIZE_ERROR	- Image height error
-	MEM_ERROR	- Memory allocation error
	
