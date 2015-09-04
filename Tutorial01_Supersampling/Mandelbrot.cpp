#include "Mandelbrot.h"

// convert iterations to colour information
inline unsigned int iterations2colour(unsigned int iter, unsigned int max_iter, unsigned int flags)
{
	// bound iterations to number of available colours
	iter = (iter * MAX_COLOURS / max_iter) & (MAX_COLOURS - 1);

	// convert iterations to colour scale based on flags (7 basic colour scales possible)
	return (((flags & 4) << 14) | ((flags & 2) << 7) | (flags & 1)) * iter;
}

// calculate Mandelbrot set
//TODO: add another variable for the number of samples per pixel
void mandelbrot(unsigned int iterations, float centrex, float centrey, float scaley, 
	unsigned int width, unsigned int height, unsigned int* out, int samples)
{
	// calculate the x distance to render based on aspect ratio of desired image and y-scale given
	float scalex = scaley * width / height;

	// calculate step size for x- and y-axis on the complex plane
	//TODO: reduce the amount to step through the complex plane proportional to the number of samples required
	float dx = (scalex / width )/ samples;
	float dy = (scaley / height)/ samples;

	// calculate top-left position on the complex plane
	float startx = centrex - scalex * 0.5f;
	float starty = centrey - scaley * 0.5f;

	// loop through entire image size (ie. (0,0) to (width - 1, height - 1))
	// and through complex plane (ie. (minx, miny) to (maxx - dx, maxy - dy))
	for (unsigned int iy = 0; iy < height; iy += 1)
	{
		for (unsigned int ix = 0; ix < width; ix += 1)
		{
			//TODO: add a variable to store the total accumulated iterations performed for all the samples
			int itcount = 0;

			//TODO: add two loops to do multiple samples for each pixel
			

			for (int ysamp = 0; ysamp < samples; ysamp++)
			{
				for (int xsamp = 0; xsamp < samples; xsamp++)
				{
					unsigned int iter = 0;
					// calculate location on the complex plane to render for the current pixel
					//TODO: offset these by the sampling loop variables above, to create sub-pixel samples
					float x0 = startx + dx * (ix * samples + xsamp);
					float y0 = starty + dy * (iy * samples + ysamp);

					// initialise complex number z represented in x and y (ie. z = (x + yi)) 
					// to be current location in complex plane 
					float x = x0;
					float y = y0;

					// iterate complex formula z = z^2 + c (c = starting location in complex plane)
					// until maximum iterations reached or |z| (distance from origin) is greater than 2
					// z^2 = (x + yi)*(x + yi) = x*x + 2*x*y*i + y*y*i*i = (x*x - y*y) + (2*x*y)i
					while (x*x + y*y < (2 * 2) && iter <= iterations)
					{
						float xtemp = x*x - y*y + x0;

						y = 2 * x*y + y0;
						x = xtemp;
						iter += 1;
					}

					
					//TODO: add sample to the total
					if (iter <= iterations)itcount += iter;
					//TODO: ensure an non-escaped (stable) calculate is correctly added to the total

					//TODO: end sampling loops
				}
			}

			// convert number of iterations to colour and store in buffer
			//TODO: average the total by the number of samples taken per pixel
			//itcount = itcount / samples;
			*out++ = iterations2colour(itcount/samples/samples, iterations, 2);
		}
	}
}

