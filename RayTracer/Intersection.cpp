/*  The following code is a VERY heavily modified from code originally sourced from:
	Ray tracing tutorial of http://www.codermind.com/articles/Raytracer-in-C++-Introduction-What-is-ray-tracing.html
	It is free to use for educational purpose and cannot be redistributed outside of the tutorial pages. */

#include "Intersection.h"

// test to see if collision between ray and a plane happens before time t (equivalent to distance)
// updates closest collision time (/distance) if collision occurs
// see: http://en.wikipedia.org/wiki/Line-sphere_intersection
// see: http://www.codermind.com/articles/Raytracer-in-C++-Part-I-First-rays.html
// see: Step 8 of http://meatfighter.com/juggler/ 
// this code make heavy use of constant term removal due to ray always being a unit vector
bool isSphereIntersected(const Sphere& s, const Ray& r, float &t)
{
    // Intersection of a ray and a sphere, check the articles for the rationale
    Vector dist = s.pos - r.start;
    float B = r.dir * dist;
    float D = B * B - dist * dist + s.size * s.size;

	// if D < 0, no intersection, so don't try and calculate the point of intersection
	if (D < 0.0f) return false;

	// calculate both intersection times(/distances)
	float t0 = B - sqrtf(D);
    float t1 = B + sqrtf(D);

	// check to see if either of the two sphere collision points are closer than time parameter
    if ((t0 > EPSILON) && (t0 < t))
    {
        t = t0;
		return true;
    } 
	else if ((t1 > EPSILON) && (t1 < t))
    {
        t = t1;
		return true;
    }

	return false;
}


// test to see if collision between ray and a plane happens before time t (equivalent to distance)
// updates closest collision time (/distance) if collision occurs
// see: http://en.wikipedia.org/wiki/Line-plane_intersection
// see: http://www.cs.princeton.edu/courses/archive/fall00/cs426/lectures/raycast/sld017.htm
// see: http://softsurfer.com/Archive/algorithm_0104/algorithm_0104B.htm#Line-Plane Intersection
bool isPlaneIntersected(const Plane& p, const Ray& r, float& t)
{
	// angle between ray and surface normal
	float angle = r.dir * p.normal;

	// no intersection if ray and plane are parallel
	if (angle == 0.0f) return false;

	// find point of intersection
	float t0 = ((p.pos - r.start) * p.normal) / angle;

	// check to see if plane collision point is closer than time parameter
	if (t0 > EPSILON && t0 < t)
	{
		t = t0;
		return true;
	}

	return false;
}


// calculate collision normal, viewProjection, object's material, and test to see if inside collision object
void calculateIntersectionResponse(const Scene& scene, const Ray& viewRay, Intersection& intersect)
{
	switch (intersect.objectType)
	{
	case Intersection::PLANE:
		intersect.normal = intersect.plane->normal;
		intersect.material = &scene.materialContainer[intersect.plane->materialId];
		break;
	case Intersection::SPHERE:
		intersect.normal = normalise(intersect.pos - intersect.sphere->pos);
		intersect.material = &scene.materialContainer[intersect.sphere->materialId];
		break;
	}

	// calculate view projection
	intersect.viewProjection = viewRay.dir * intersect.normal; 

	// detect if we are inside an object (needed for refraction)
	intersect.insideObject = (intersect.normal * viewRay.dir > 0.0f) && (intersect.objectType != Intersection::PLANE);

	// if inside an object, reverse the normal
    if (intersect.insideObject)
    {
        intersect.normal = -1.0f * intersect.normal;
    }
}


// test to see if collision between ray and any object in the scene
// updates intersection structure if collision occurs
bool objectIntersection(const Scene& scene, const Ray& viewRay, Intersection& intersect)
{
	// set default distance to be a long long way away
    float t = MAX_RAY_DISTANCE;

	// no intersection found by default
	intersect.objectType = Intersection::NONE;

	// search for plane collisions, storing closest one found
    for (unsigned int i = 0; i < scene.numPlanes; ++i)
    {
        if (isPlaneIntersected(scene.planeContainer[i], viewRay, t))
        {
			intersect.objectType = Intersection::PLANE;
			intersect.plane = &scene.planeContainer[i];
        }
    }

	// search for sphere collisions, storing closest one found
    for (unsigned int i = 0; i < scene.numSpheres; ++i)
    {
        if (isSphereIntersected(scene.sphereContainer[i], viewRay, t))
        {
			intersect.objectType = Intersection::SPHERE;
			intersect.sphere = &scene.sphereContainer[i];
        }
    }

	// nothing detected, return false
	if (intersect.objectType == Intersection::NONE)
	{
		return false;
	}

	// calculate the point of the intersection
	intersect.pos = viewRay.start + t * viewRay.dir;

	return true;
}
