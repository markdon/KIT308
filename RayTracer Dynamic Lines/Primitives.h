/*  The following code is a VERY heavily modified from code originally sourced from:
	Ray tracing tutorial of http://www.codermind.com/articles/Raytracer-in-C++-Introduction-What-is-ray-tracing.html
	It is free to use for educational purpose and cannot be redistributed outside of the tutorial pages. */

#ifndef __PRIMITIVES_H
#define __PRIMITIVES_H

#include <cmath>

#include "Constants.h"


// points consist of three coordinates and represent a point in 3d space
struct Point 
{
	float x, y, z;
};


// vectors consist of three coordinates and represent a direction from (an implied) origin
struct Vector 
{
	float x, y, z;

    Vector& operator += (const Vector& v2) 
	{
	    this->x += v2.x;
        this->y += v2.y;
        this->z += v2.z;
	    return *this;
    }

	inline float dot() const 
	{
		return x * x + y * y + z * z;
	}
};


// point + vector (produces a point)
inline Point operator + (const Point& p, const Vector& v) 
{
	Point p2 = { p.x + v.x, p.y + v.y, p.z + v.z };
	return p2;
}

// point - vector (produces a point)
inline Point operator - (const Point& p, const Vector& v)
{
	Point p2 = { p.x - v.x, p.y - v.y, p.z - v.z };
	return p2;
}

// vector + vector
inline Vector operator + (const Vector& v1, const Vector& v2)
{
	Vector v = { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
	return v;
}

// point - point (produces a vector)
inline Vector operator - (const Point& p1, const Point& p2)
{
	Vector v = { p1.x - p2.x, p1.y - p2.y, p1.z - p2.z };
	return v;
}

// float * vector (float is multiplied by each component of the vector)
inline Vector operator * (float c, const Vector& v)
{
	Vector v2 = { v.x *c, v.y * c, v.z * c };
	return v2;
}

// vector - vector
inline Vector operator - (const Vector& v1, const Vector& v2)
{
	Vector v = { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
	return v;
}

// vector * vector
inline float operator * (const Vector& v1, const Vector& v2 ) 
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

// helper function
inline float invsqrtf(const float& x) 
{ 
	return 1.0f / sqrtf(x); 
}

// normalise the vector (ie. make the vector's magnitude equal to 1)
// not to be confused with the normal vector of a surface/intersection
inline Vector normalise(const Vector& x)
{
	return invsqrtf(x.dot()) * x;
}


// rays are cast from a starting point in a direction
struct Ray 
{
	Point start;
	Vector dir;
};


#endif //__PRIMITIVES_H
