/*  The following code is a VERY heavily modified from code originally sourced from:
	Ray tracing tutorial of http://www.codermind.com/articles/Raytracer-in-C++-Introduction-What-is-ray-tracing.html
	It is free to use for educational purpose and cannot be redistributed outside of the tutorial pages. */

#define TARGET_WINDOWS

#include "Timer.h"
#include "Primitives.h"
#include "Scene.h"
#include "Lighting.h"
#include "Intersection.h"
#include "ImageIO.h"

unsigned int buffer[MAX_WIDTH * MAX_HEIGHT];

//ADDED: ThreadData struct
typedef struct {
	Scene scene;			//The scene to render.
	unsigned int threadID;	//Sequencial ID of thread.
	unsigned int samples;	//Raytrace samples per pixel (aaLevel)
	unsigned int width;		//The amount of pixels in each row
	unsigned int height;	//Amount of rows this thread will render
	unsigned int* outStart;	//Pointer to position in buffer to start writing
	int yOffset;			//Amount of lines to offset the scene render area for this thread
} ThreadData;



// reflect the ray from an object
Ray calculateReflection(const Ray& viewRay, const Intersection& intersect) 
{
	// reflect the viewRay around the object's normal
	Ray newRay = { intersect.pos, viewRay.dir - (2.0f * intersect.viewProjection * intersect.normal) };

	return newRay;
}


// refract the ray through an object
Ray calculateRefraction(const Ray& viewRay, const Intersection& intersect, float& currentRefractiveIndex)
{
	// change refractive index depending on whether we are in an object or not
	float oldRefractiveIndex = currentRefractiveIndex;
	currentRefractiveIndex = intersect.insideObject ? DEFAULT_REFRACTIVE_INDEX : intersect.material->density;

	// calculate refractive ratio from old index and current index
	float refractiveRatio = oldRefractiveIndex / currentRefractiveIndex;

	// Here we take into account that the light movement is symmetrical from the observer to the source or from the source to the oberver.
	// We then do the computation of the coefficient by taking into account the ray coming from the viewing point.
	float fCosThetaT, fCosThetaI = fCosThetaI = fabsf(intersect.viewProjection); 

	// glass-like material, we're computing the fresnel coefficient.
	if (fCosThetaI >= 1.0f) 
	{
		// In this case the ray is coming parallel to the normal to the surface
		fCosThetaT = 1.0f;
	}
	else 
	{
		float fSinThetaT = refractiveRatio * sqrtf(1 - fCosThetaI * fCosThetaI);

		// Beyond the angle (1.0f) all surfaces are purely reflective
		fCosThetaT = (fSinThetaT * fSinThetaT >= 1.0f) ? 0.0f : sqrtf(1 - fSinThetaT * fSinThetaT);
	}

    // Here we compute the transmitted ray with the formula of Snell-Descartes
	Ray newRay = { intersect.pos, refractiveRatio * (viewRay.dir + fCosThetaI * intersect.normal) - (fCosThetaT * intersect.normal) };

	return newRay;
}


// follow a single ray until it's final destination (or maximum number of steps reached)
Colour traceRay(const Scene& scene, Ray viewRay) 
{
    Colour output(0.0f, 0.0f, 0.0f); 								// colour value to be output
	float currentRefractiveIndex = DEFAULT_REFRACTIVE_INDEX;		// current refractive index
    float coef = 1.0f;												// amount of ray left to transmit
	Intersection intersect;											// properties of current intersection

	// loop until reached maximum ray cast limit (unless loop is broken out of)
    for (int level = 0; level < MAX_RAYS_CAST; ++level)
    {
		// check for intersections between the view ray and any of the objects in the scene
		// exit the loop if no intersection found
		if (!objectIntersection(scene, viewRay, intersect)) break;

		// calculate response to collision: ie. get normal at point of collision and material of object
		calculateIntersectionResponse(scene, viewRay, intersect);

		// apply the diffuse and specular lighting 
		if (!intersect.insideObject) output += coef * applyLighting(scene, viewRay, intersect);

		// if object has reflection or refraction component, adjust the view ray and coefficent of calculation and continue looping
		if (intersect.material->reflection)
		{
			viewRay = calculateReflection(viewRay, intersect); 
			coef *= intersect.material->reflection;
		}
		else if (intersect.material->refraction)
		{
			viewRay = calculateRefraction(viewRay, intersect, currentRefractiveIndex); 
			coef *= intersect.material->refraction;
		}
		else
		{
			// if no reflection or refraction, then finish looping (cast no more rays)
			return output;
		}
    }

	// if the calculation coefficient is non-zero, read from the environment map
    if (coef > 0.0f)
    {
		Material& currentMaterial = scene.materialContainer[scene.skyboxMaterialId];

		// use the cubemap if there is one, otherwise just apply the diffuse colour
		if (currentMaterial.wrapping == Material::CUBEMAP)
		{
			output += coef * readCubemap(currentMaterial.cubemap, viewRay.dir);
		}
		else
		{
			output += coef * currentMaterial.diffuse;
		}
    }

    return output;
}


// render scene at given width and height and anti-aliasing level
//ADDED: outStart parameter specifies where output should start
void render(Scene& scene, const int width, const int height, const int aaLevel, unsigned int* outStart, unsigned int threadID, int yOffset)
{
	// angle between each successive ray cast (per pixel, anti-aliasing uses a fraction of this)
	const float dirStepSize = 1.0f / (0.5f * width / tanf(PIOVER180 * 0.5f * scene.cameraFieldOfView));

	// pointer to output buffer
	unsigned int* out = outStart;

	// loop through all the pixels
	for (int y = (-height / 2) + yOffset; y < (height / 2) + yOffset; ++y)
	{
		for (int x = -width / 2; x < width / 2; ++x)
		{

			//Changing these values 
			//Colour output(threadID * threadID * threadID);
			Colour output(unsigned int(0));

			// calculate multiple samples for each pixel
			const float sampleStep = 1.0f / aaLevel, sampleRatio = 1.0f / (aaLevel * aaLevel);
			
			// loop through all sub-locations within the pixel
			for (float fragmentx = float(x); fragmentx < x + 1.0f; fragmentx += sampleStep) //1.0f / aaLevel)
			{
				for (float fragmenty = float(y); fragmenty < y + 1.0f; fragmenty += sampleStep) //1.0f / aaLevel)
				{
					// direction of default forward facing ray
					Vector dir = { fragmentx * dirStepSize, fragmenty * dirStepSize, 1.0f }; 

					// rotated direction of ray
					Vector rotatedDir = { 
						dir.x * cosf(scene.cameraRotation) - dir.z * sinf(scene.cameraRotation), 
						dir.y, 
						dir.x * sinf(scene.cameraRotation) + dir.z * cosf(scene.cameraRotation) };

					// view ray starting from camera position and heading in rotated (normalised) direction
					Ray viewRay = { scene.cameraPosition, normalise(rotatedDir) };

					// follow ray and add proportional of the result to the final pixel colour
					output += sampleRatio * traceRay(scene, viewRay);
				}
			}

			// store saturated final colour value in image buffer
			*out++ = output.convertToPixel(scene.exposure);
		}
	}
}

/*	The function called for each thread.
Starts a render() function with parameters from threadData
*/
DWORD __stdcall StartThread(void* threadDataIn)
{
	ThreadData* threadData = (ThreadData*)threadDataIn;

	/*unsigned int* out = threadData->outStart;
	for (unsigned int y = 0; y < threadData->height; y++) {
	for (unsigned int x = 0; x < threadData->width; x++) {
	*out++ = x + y;

	}

	}*/
	printf("Thread: %d", threadData->threadID);
	render(threadData->scene, threadData->width, threadData->height, threadData->samples, threadData->outStart, threadData->threadID, threadData->yOffset);

	ExitThread(NULL);
}

/*	ADDED
threading()
Initialise variables for each ThreadData and begin the thread	*/
void threading(Scene& scene, const int width, const int height, const int samples, bool colourise, unsigned int threads) {

	HANDLE *threadHandles = new HANDLE[threads];	//Create array of thread handles
	ThreadData *tDatas = new ThreadData[threads];	//Create array of ThreadData structs

	unsigned int threadHeight = (height / threads) - (height / threads) % 2; //The height of most threads. Rounded down to even numbers so the render loop's divide by 2 works correctly
	unsigned int threadID;
	for (threadID = 0; threadID < threads -1; threadID++) {
		tDatas[threadID].threadID = threadID;
		tDatas[threadID].scene = scene;
		tDatas[threadID].samples = samples;
		tDatas[threadID].width = width;
		tDatas[threadID].height = threadHeight;
		tDatas[threadID].outStart = buffer + width * threadHeight * threadID;	//Output start point is beginning of buffer plus however many pixels are being rendered by earlier threads
		tDatas[threadID].yOffset = threadHeight * (threadID - threads / 2);		//Calculate vertical offset from center of scene for this thread to render
		threadHandles[threadID] = CreateThread(NULL, 0, StartThread, &tDatas[threadID], 0, NULL);
	}
	//Last thread completes the lines left from removing any odd lines when height doesn't evenly divide by threads in addition to the amount completed by other threads
	unsigned int oddLines = height - threadHeight * threads;
	tDatas[threadID].threadID = threadID;
	tDatas[threadID].scene = scene;
	tDatas[threadID].samples = samples;
	tDatas[threadID].width = width;
	tDatas[threadID].height = threadHeight + oddLines;
	tDatas[threadID].outStart = buffer + width * threadHeight * threadID;				
	tDatas[threadID].yOffset = threadHeight * (threadID - threads / 2) + oddLines/2;	
	threadHandles[threadID] = CreateThread(NULL, 0, StartThread, &tDatas[threadID], 0, NULL);

	WaitForMultipleObjects(threads, threadHandles, TRUE, INFINITE);

}

// read command line arguments, render, and write out BMP file
int main(int argc, char* argv[])
{
	int width = 1024;
	int height = 1024;
	int samples = 1;
	char* inputFilename = "Scenes/cornell.txt";
	char* outputFilename = "Outputs/cornell-1024x1024x1.bmp";

	// rendering options
	unsigned int threads = 1;			
	bool colourise = false;				// currently unused
	unsigned int blockSize = -1;		// curerntly unused

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-size") == 0)
		{
			width = atoi(argv[++i]);
			height = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-samples") == 0)
		{
			samples = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-input") == 0)
		{
			inputFilename = argv[++i];
		}
		else if (strcmp(argv[i], "-output") == 0)
		{
			outputFilename = argv[++i];
		}
		else if (strcmp(argv[i], "-threads") == 0)
		{
			threads = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-colourise") == 0)
		{
			colourise = true;
		}
		else if (strcmp(argv[i], "-blockSize") == 0)
		{
			blockSize = atoi(argv[++i]);
		}
		else
		{
			fprintf(stderr, "unknown argument: %s\n", argv[i]);
		}
	}

	// read scene file
    Scene scene;
    if (!init(inputFilename, scene))
    {
        fprintf(stderr, "Failure when reading the Scene file.\n");
        return -1;
    }

	Timer timer;									// create timer
	

	//render(scene, width, height, samples);			// raytrace scene
	threading(scene, width, height, samples, colourise, threads);									//ADDED: threading(); Begin thread creation	
	timer.end();									// record end time

	// output timing information
	printf("time taken: %ums\n", timer.getMilliseconds());

	// output BMP file
	write_bmp(outputFilename, buffer, width, height, width);
	//getchar(); //pause

	return 0;
}

