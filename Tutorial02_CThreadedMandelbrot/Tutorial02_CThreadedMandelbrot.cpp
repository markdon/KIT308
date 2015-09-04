// Tutorial02_CThreadedMandelbrot.cpp 
// Basic multithreaded (CPU) Mandelbrot generation with supersampling

//TODO: include the header that contains Thread functions
#include <windows.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bmp.h"
#include "Mandelbrot.h"

// buffer for colour information
unsigned int buffer[MAX_WIDTH * MAX_HEIGHT];

//TODO: declare a ThreadData (or similarly named) structure to hold all the parameters necessary for running the mandelbrot function
typedef struct {
	unsigned int threadID;
	unsigned int iterations;
	float centrex;
	float centrey;
	float scaley;
	unsigned int samples;
	unsigned int width;
	unsigned int height;
	unsigned int* out;
} ThreadData;

//TODO: declare a THREAD_START_ROUTINE function that casts its argument, calls the mandelbrot function, and then exits gracefully
DWORD __stdcall StartThread(void* threadDataIn)
{
	ThreadData* threadData = (ThreadData*) threadDataIn;
	//for (int i = 0; i < 50; i++)printf("%d", threadData->threadID);
	//printf("(%d)", threadData->threadID);
	printf("Thread: %d %d %f %f %f %d %d %d %d \n", threadData->threadID, threadData->iterations, threadData->centrex, threadData->centrey, threadData->scaley, threadData->samples, threadData->width, threadData->height, threadData->out);
	mandelbrot(threadData->threadID, threadData->iterations, threadData->centrex, threadData->centrey, threadData->scaley, threadData->samples, threadData->width, threadData->height, threadData->out);
	
	ExitThread(NULL);
}

static void generate(unsigned int threads, unsigned int iterations, float centrex, float centrey, float scaley,
	unsigned int samples, unsigned int width, unsigned int height, unsigned int* out)
{
	// output (hopefully helpful) generation message
	printf("generating with %d threads at (%f, %f) with scale %f and %d iterations at size %dx%d with %d samples per pixel\n", 
		threads, centrex, centrey, scaley, iterations, width, height, samples * samples);

	// timing variables
	clock_t start, finish;

	// calculate mandelbrot and time how long it takes
	start = clock();

	//TODO: declare and create an array of HANDLEs dynamically (using new)  
	HANDLE *threadHandles = new HANDLE[threads];

	//TODO: declare and create an array of ThreadDatas dynamically (using new)  
	ThreadData *tDatas = new ThreadData[threads];

	//mandelbrot(6, iterations, centrex, centrey, scaley, samples, width, height, out);
	//TODO: remove the above line and instead add a loop to create the Threads 
	for (unsigned int i = 0; i < threads; i++){
		
		//TODO: initialise the corresponding ThreadData object remembering that each Thread should:
		tDatas[i].threadID = i;
		//TODO: - calculate a different part of the Mandelbrot image (i.e. each Thread's centrey and scaley need to be different from the function argument) 
		tDatas[i].centrey = centrey - (scaley / threads)*(threads - 1)*0.5f + (scaley / threads)*i;
		tDatas[i].scaley = scaley / threads;
		//TODO: - render less than the whole height of the image (i.e. the height needs to be different from the function argument)
		tDatas[i].height = height / threads;
		//TODO: - store the results of the render in a different place in memory (i.e. the out pointer needs to be different from the function argument)
		tDatas[i].out = out + i*width*(height / threads);
		//TODO: create a Thread and store the returned HANDLE
		//printf("Created ThreadData: %d\n", newThread->threadID);
		tDatas[i].iterations = iterations;
		tDatas[i].centrex = centrex;
		tDatas[i].samples = samples;
		tDatas[i].width = width;
		threadHandles[i] = CreateThread(NULL, 0, StartThread, &tDatas[i], 0, NULL);
	}
	
		

	//TODO: wait for all the Threads to finish
	WaitForMultipleObjects(threads, threadHandles, TRUE, INFINITE);
	//TODO: delete the dynamically declared arrays

	finish = clock();

	printf("time taken: %ums\n", finish - start);
}

int main(int argc, char **argv)
{
	// number of threads to run on
	unsigned int threads = DEFAULT_THREADS;

	// size of output
	unsigned int width = DEFAULT_WIDTH, height = DEFAULT_HEIGHT;

	// "accuracy" of calculation
	unsigned int iterations = DEFAULT_MAX_ITER;

	// area of interest (centre coordinates on the complex plane and height of area)
	float centrex = DEFAULT_CENTRE_X;
	float centrey = DEFAULT_CENTRE_Y;
	float scaley = DEFAULT_SCALE_Y;

	unsigned int samples = DEFAULT_SAMPLES;

	// read any command-line arguments
	for (int i = 1; i < argc; i++) 
	{
		if (strcmp(argv[i], "-iterations") == 0)
		{
			iterations = atoi(argv[++i]);
		} 
		else if (strcmp(argv[i], "-size") == 0)
		{
			width = atoi(argv[++i]);
			height = atoi(argv[++i]);
		} 
		else if (strcmp(argv[i], "-centre") == 0)
		{
			centrex = (float) atof(argv[++i]);
			centrey = (float) atof(argv[++i]);
		}
		else if (strcmp(argv[i], "-scale") == 0)
		{
			scaley = (float) atof(argv[++i]);
		}
		else if (strcmp(argv[i], "-samples") == 0)
		{
			samples = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-threads") == 0)
		{
			threads = atoi(argv[++i]);
		}
		else
		{
			fprintf(stderr, "unknown argument: %s\n", argv[i]);
		}
	}

	// generate Mandelbrot fractal in global buffer
	generate(threads, iterations, centrex, centrey, scaley, samples, width, height, buffer);

	// output (uncompressed) bmp file
	write_bmp("output.bmp", width, height, (char*) buffer);

	// success!
	return 0;
}

