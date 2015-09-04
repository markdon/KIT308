/*  The following code is a VERY heavily modified from code originally sourced from:
	Ray tracing tutorial of http://www.codermind.com/articles/Raytracer-in-C++-Introduction-What-is-ray-tracing.html
	It is free to use for educational purpose and cannot be redistributed outside of the tutorial pages. */

#ifndef __TEXTURE_H
#define __TEXTURE_H

#include "Primitives.h"
#include "Colour.h"

// a texture retains its width, height, image data (as a 1d array of pixels in 0x00BBGGRR),
// and properties of its colouring
struct Texture
{
	int width, height;					// image size
	unsigned int* data;					// image data
	bool exposed;						// whether or not the image was pre-exposed
	bool sRGB;							// whether or not the image is in sRGB format
};

// a cubemap consists of six textures representing the face of the cube
struct CubeMap
{
	enum { UP = 0, DOWN = 1, RIGHT = 2, LEFT = 3, FORWARD = 4, BACKWARD = 5 };

	Texture textures[6];
};

// read a pixel (using bilinear interpolation) using real coordinates of the range [0, 1]
Colour readTexture(const Texture& t, float u, float v);

// read a pixel (using bilinear interpolation) from the cubemap location pointed to by the ray direction
Colour readCubemap(const CubeMap& cm, const Vector& ray);

#endif  //__TEXTURE_H
