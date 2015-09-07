/*  The following code is a VERY heavily modified from code originally sourced from:
	Ray tracing tutorial of http://www.codermind.com/articles/Raytracer-in-C++-Introduction-What-is-ray-tracing.html
	It is free to use for educational purpose and cannot be redistributed outside of the tutorial pages. */

#ifndef __SCENE_OBJECTS_H
#define __SCENE_OBJECTS_H

#include "Primitives.h"
#include "Texture.h"


// material
struct Material
{
	// type of colouring
	enum { GOURAUD, CHECKERBOARD, TEXTURE } type;

	// type of colour wrapping
	// gourard types ignore all wrappings
	// cubemap isn't compatible with checkerboard
	// texture isn't compatible with volume
	enum { PLANAR_XY, PLANAR_XZ, PLANAR_YZ, VOLUME, SPHERICAL, CUBEMAP } wrapping;

	Colour diffuse;				// diffuse colour
	Colour diffuse2;			// second diffuse colour, only for checkerboard types

	float size;					// size of texture or checkboard (only for planar wrappings)

	Colour specular;			// colour of specular lighting
	float power;				// power of specular reflection

	float reflection;			// reflection amount
	float refraction;			// refraction amount
	float density;				// density of material (affects amount of defraction)

	struct Texture texture;		// single texture
	struct CubeMap cubemap;		// cubemap texture
};

// sphere object
struct Sphere 
{
	Point pos;					// a point on the plane
	float size;					// radius of sphere
	unsigned int materialId;	// material id
};

// plane object
struct Plane
{
	Point pos;					// a point on the plane
	Vector normal;				// normal of the plane
	unsigned int materialId;	// material id
};

// light object
struct Light 
{
	Point pos;					// location
	Colour intensity;			// brightness and colour
};

#endif // __SCENE_OBJECTS_H
