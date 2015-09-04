// Tutorial03c_DynamicLinesViaSharedMemory.cpp 
// Multithreaded (CPU) Mandelbrot generation with dynamic load balancing via shared memory

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bmp.h"
#include "Mandelbrot.h"

// buffer for colour information
unsigned int buffer[MAX_WIDTH * MAX_HEIGHT];

// ThreadData structure to hold all the parameters necessary for running the mandelbrot function
struct ThreadData 
{
	unsigned int threadID;
	unsigned int iterations;
	float centrex;
	float centrey;
	float scaley;
	unsigned int samples;
	unsigned int width;
	unsigned int height;
	unsigned int* out;
	//TODO: store extra parameter (pointer to a shared memory location) DONE
	int* nextStart;
};

// a THREAD_START_ROUTINE function that casts its argument, calls the mandelbrot function, and then exits gracefully
DWORD __stdcall mandelbrotThreadStart(LPVOID threadData)
{
	// cast the pointer to void (i.e. an untyped pointer) into something we can use
	ThreadData* data = (ThreadData*) threadData;

	//TODO: pass extra parameter through DONE
	mandelbrot(data->threadID, data->iterations, data->centrex, data->centrey, data->scaley, data->samples, data->width, data->height, data->out, data->nextStart);

	ExitThread(NULL);
}

static void generate(unsigned int threads, unsigned int iterations, float centrex, float centrey, float scaley,
	unsigned int samples, unsigned int width, unsigned int height, unsigned int* out)
{
	// output (hopefully helpful) generation message
	printf("generating at (%f, %f) with scale %f and %d iterations at size %dx%d with %d samples per pixel\n", 
		centrex, centrey, scaley, iterations, width, height, samples * samples);

	// timing variables
	clock_t start, finish;

	// calculate mandelbrot and time how long it takes
	start = clock();

	// dynamically sized storage for Thread handles and initialisation data
	HANDLE* threadHandles = new HANDLE[threads];
	ThreadData* threadData = new ThreadData[threads];

	//TODO: declare a variable to act as shared memory for the threads
	int nextStart = -1;
	//TODO: this variable should represent one less than the number of lines done so far (because we are using InterlockedIncrement)

	// create all the threads with sensible initial values
	for (unsigned int i = 0; i < threads; i++) {
		threadData[i].threadID = i;
		threadData[i].iterations = iterations;
		threadData[i].centrex = centrex;
		threadData[i].centrey = centrey;
		threadData[i].scaley = scaley;
		threadData[i].samples = samples;
		threadData[i].width = width;
		threadData[i].height = height;
		threadData[i].out = out;
		//TODO: set pointer to shared memory in ThreadData structure
		threadData[i].nextStart = &nextStart;

		threadHandles[i] = CreateThread(NULL, 0, mandelbrotThreadStart, (void*) &threadData[i], 0, NULL);
	}

	// wait for everything to finish
	for (unsigned int i = 0; i < threads; i++) {
		WaitForSingleObject(threadHandles[i], INFINITE);
	}
	//WaitForMultipleObjects(threads, threadHandles, true, INFINITE); // WaitForMultipleObjects will work, but has a limit of 64 threads

	// release dynamic memory
	delete[] threadHandles;
	delete[] threadData;

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

