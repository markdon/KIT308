#ifndef _MANDELBROT_H
#define _MANDELBROT_H

// Mandelbrot defaults and limits
#define MAX_WIDTH 4096
#define MAX_HEIGHT 4096
#define MAX_COLOURS 256				// must be a power of 2
#define DEFAULT_WIDTH 1024			// no bigger than MAX_WIDTH
#define DEFAULT_HEIGHT 1024			// no bigger than MAX_HEIGHT

#define DEFAULT_MAX_ITER 256
#define DEFAULT_CENTRE_X -0.75f
#define DEFAULT_CENTRE_Y 0.0f
#define DEFAULT_SCALE_Y 2.5f
#define DEFAULT_SAMPLES 1

//TODO: add another default value for the number of samples per pixel

//TODO: add another argument (samples per pixel) to the prototype
void mandelbrot(unsigned int iterations, float centrex, float centrey, float scaley, 
	unsigned int width, unsigned int height, unsigned int* out, int samples);

#endif
