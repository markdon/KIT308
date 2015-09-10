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
bool colourise;						//Tint pixels when rendering.
unsigned int colourIntensity;		//Intensity of pixel tinting.

//ADDED: ThreadData struct
typedef struct {
	Scene scene;			//The scene to render.
	unsigned int threadID;	//Sequencial ID of thread.
	unsigned int samples;	//Raytrace samples per pixel (aaLevel)
	unsigned int width;		//The amount of pixels in each row
	unsigned int height;
	unsigned int* out;		//Pointer to position in buffer to write pixel
	unsigned int* lineCount;		//Pointer to number of lines rendered
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
void render(Scene& scene, const int width, const int height, const int aaLevel, unsigned int threadID, unsigned int* lineCount)
{
	// angle between each successive ray cast (per pixel, anti-aliasing uses a fraction of this)
	const float dirStepSize = 1.0f / (0.5f * width / tanf(PIOVER180 * 0.5f * scene.cameraFieldOfView));

	unsigned int threadMod7 = threadID % 7; //Assign number out of 7 for thread colour

	unsigned int line;
	while ((line = InterlockedIncrement(lineCount)) < height)
	{
		// loop through all the pixels
		//tDatas[index].sceneYCoord = line - height / 2;
		int y = line - height / 2;
		for (int x = -width / 2; x < width / 2; ++x)
		{
			

			/*	Set thread colour tint
				Bitwise logic: threadMod7 will have up to all lowest 3 bits on.
				Take third bit, left shifts into lowest bit in blue byte, OR into next.
				Take second bit, left shifts into lowest bit in green byte, OR into next.
				Take third bit, left shifts into lowest bit in red byte.
				This number is then multiplied to amplify the turned on colours.
			*/
			unsigned int colour = 0;
			if (colourise) colour = (((threadMod7 & 4) << 14) | ((threadMod7 & 2) << 7) | (threadMod7 & 1)) * colourIntensity;
			Colour output(colour);


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

			// store saturated final colour value in image buffer. x is translated from scene coords to pixel coords.
			buffer[(x + width/2) + width * line] = output.convertToPixel(scene.exposure);
		}
	}
}

/*	The function called for each thread.
Starts a render() function with parameters from threadData
*/
DWORD __stdcall StartThread(void* threadDataIn)
{
	ThreadData* threadData = (ThreadData*)threadDataIn;
	render(threadData->scene, threadData->width, threadData->height, threadData->samples, threadData->threadID, threadData->lineCount);

	ExitThread(NULL);
}

/*	ADDED
threading()
Initialise variables for each ThreadData and begin the thread	*/
void threading(Scene& scene, const int width, const int height, const int samples, bool colourise, unsigned int threads) {

	HANDLE *threadHandles = new HANDLE[threads];	//Create array of thread handles
	ThreadData *tDatas = new ThreadData[threads];	//Create array of ThreadData structs
	unsigned int lineCount = -1;					//Count of number of lines rendered. Incremented by threads as they start a line.

	//Create all the threads and give starting values
	for (unsigned int i = 0; i < threads; i++) {
		tDatas[i].threadID = i;
		tDatas[i].scene = scene;
		tDatas[i].samples = samples;
		tDatas[i].width = width;
		tDatas[i].height = height;
		tDatas[i].lineCount = &lineCount;
		threadHandles[i] = CreateThread(NULL, 0, StartThread, &tDatas[i], 0, NULL);
	}

	for (unsigned int i = 0; i < threads; i++) {
		WaitForSingleObject(threadHandles[i], INFINITE);
	}
	delete[] threadHandles;
	delete[] tDatas;

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
	colourise = false;
	colourIntensity = 150;
	unsigned int threads = 1;			
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
		else if (strcmp(argv[i], "-colourIntensity") == 0)
		{
			colourIntensity = atoi(argv[++i]);
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
	threading(scene, width, height, samples, colourise, threads);	//Start thread creation.
	timer.end();									// record end time

	// output timing information
	printf("%ums\t", timer.getMilliseconds());

	// output BMP file
	write_bmp(outputFilename, buffer, width, height, width);
	//getchar(); //pause

	return 0;
}

