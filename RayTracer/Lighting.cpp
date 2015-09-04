/*  The following code is a VERY heavily modified from code originally sourced from:
	Ray tracing tutorial of http://www.codermind.com/articles/Raytracer-in-C++-Introduction-What-is-ray-tracing.html
	It is free to use for educational purpose and cannot be redistributed outside of the tutorial pages. */

#include "Lighting.h"
#include "Intersection.h"

// test to see if light ray collides with any of the scene's objects
// short-circuits when first intersection discovered, because no matter what the object will be in shadow
bool isInShadow(const Scene& scene, const Ray& lightRay, const float lightDist)
{
    float t = lightDist;

	// search for plane collision
	for (unsigned int i = 0; i < scene.numPlanes; ++i)
    {
        if (isPlaneIntersected(scene.planeContainer[i], lightRay, t))
        {
			return true;
		}
    }

	// search for sphere collision
    for (unsigned int i = 0; i < scene.numSpheres; ++i)
    {
        if (isSphereIntersected(scene.sphereContainer[i], lightRay, t))
        {
            return true;
        }
    }

	// not in shadow
	return false;
}


// apply computed checkerboard texture with respect to material's wrapping style
Colour applyCheckerboard(const Intersection& intersect)
{
	int which;

	switch (intersect.material->wrapping)
	{
	// planar wrapping uses only two of the three coordinates
	case Material::PLANAR_XY:
		which = (int(floorf(intersect.pos.x / intersect.material->size)) + int(floorf(intersect.pos.y / intersect.material->size))) & 1;
		break;
	case Material::PLANAR_XZ:
		which = (int(floorf(intersect.pos.x / intersect.material->size)) + int(floorf(intersect.pos.z / intersect.material->size))) & 1;
		break;
	case Material::PLANAR_YZ:
		which = (int(floorf(intersect.pos.y / intersect.material->size)) + int(floorf(intersect.pos.z / intersect.material->size))) & 1;
		break;
	// volume wrapping uses all three coordinates (can be grainy if checkerboard pattern lines up with object boundary)
	case Material::VOLUME:
		which = (int(floorf(intersect.pos.x / intersect.material->size)) + int(floorf(intersect.pos.y / intersect.material->size)) + int(floorf(intersect.pos.z / intersect.material->size))) & 1;
		break;
	// spherical wrapping with the poles on the y axis
	case Material::SPHERICAL:
		{
			float u = atan2f(intersect.normal.z, intersect.normal.x) / (PI * 2) + 0.5f;
			float v = asinf(intersect.normal.y) / PI + 0.5f;

			which = (int(u * (intersect.material->size * 2)) + int(v * intersect.material->size)) & 1;
		}
		break;
	}

	return (which ? intersect.material->diffuse : intersect.material->diffuse2);
}


// apply texture with respect to material's wrapping style
Colour applyTexture(const Intersection& intersect)
{
	float u, v;

	switch (intersect.material->wrapping)
	{
	// planar wrapping uses only two of the three coordinates
	case Material::PLANAR_XY:
		u = fmodf(intersect.pos.x, intersect.material->size) / intersect.material->size;
		v = fmodf(intersect.pos.y, intersect.material->size) / intersect.material->size;
		break;
	case Material::PLANAR_XZ:
		u = fmodf(intersect.pos.x, intersect.material->size) / intersect.material->size;
		v = fmodf(intersect.pos.z, intersect.material->size) / intersect.material->size;
		break;
	case Material::PLANAR_YZ:
		u = fmodf(intersect.pos.y, intersect.material->size) / intersect.material->size;
		v = fmodf(intersect.pos.z, intersect.material->size) / intersect.material->size;
		break;
	// spherical wrapping with the poles on the y axis
	case Material::SPHERICAL:
		u = atan2f(intersect.normal.z, intersect.normal.x) / (PI * 2) + 0.5f;
		v = asinf(intersect.normal.y) / PI + 0.5f;
		break;
	// follow normal until intersection with cubemap
	case Material::CUBEMAP:
		return readCubemap(intersect.material->cubemap, intersect.normal);
	}

	return readTexture(intersect.material->texture, u, v);
}


// apply diffuse lighting with respect to material's colouring
Colour applyDiffuse(const Ray& lightRay, const Light& currentLight, const Intersection& intersect) 
{
	Colour output;

    switch (intersect.material->type)
	{
	case Material::CHECKERBOARD:
		output = applyCheckerboard(intersect);
		break;
	case Material::TEXTURE:
		output = applyTexture(intersect);
		break;
	case Material::GOURAUD:
		output = intersect.material->diffuse;
        break;
    }

    float lambert = lightRay.dir * intersect.normal; 

	return lambert * currentLight.intensity * output;
}

// Blinn 
// The direction of Blinn is exactly at mid point of the light ray and the view ray. 
// We compute the Blinn vector and then we normalize it then we compute the coeficient of blinn
// which is the specular contribution of the current light.
Colour applySpecular(const Ray& lightRay, const Light& currentLight, const float fLightProjection, const Ray& viewRay, const Intersection& intersect) 
{
	Vector blinnDir = lightRay.dir - viewRay.dir;
	float blinn = invsqrtf(blinnDir.dot()) * std::max(fLightProjection - intersect.viewProjection, 0.0f);
	blinn = powf(blinn, intersect.material->power);

	return blinn * intersect.material->specular * currentLight.intensity;
}


// apply diffuse and specular lighting contributions for all lights in scene taking shadowing into account
Colour applyLighting(const Scene& scene, const Ray& viewRay, const Intersection& intersect) 
{
	// colour to return (starts as black)
	Colour output(0.0f, 0.0f, 0.0f);

	// same starting point for each light ray
	Ray lightRay = { intersect.pos };

	// loop through all the lights
    for (unsigned int j = 0; j < scene.numLights; ++j)
    {
		// get reference to current light
		const Light& currentLight = scene.lightContainer[j];

		// light ray direction need to equal the normalised vector in the direction of the current light
		// as we need to reuse all the intermediate components for other calculations, 
		// we calculate the normalised vector by hand instead of using the normalise function
		lightRay.dir = currentLight.pos - intersect.pos;
		float angleBetweenLightAndNormal = lightRay.dir * intersect.normal;
		
		// skip this light if it's behind the object (ie. both light and normal pointing in the same direction)
		if (angleBetweenLightAndNormal <= 0.0f)
		{
			continue;
		}

		// distance to light from intersection point (and it's inverse)
		float lightDist = sqrtf(lightRay.dir.dot());
		float invLightDist = 1.0f / lightDist;

		// light ray projection
		float lightProjection = invLightDist * angleBetweenLightAndNormal;

		// normalise the light direction
		lightRay.dir = invLightDist * lightRay.dir;

		// only apply lighting from this light if not in shadow of some other object
		if (!isInShadow(scene, lightRay, lightDist))
		{
			// add diffuse lighting from colour / texture
			output += applyDiffuse(lightRay, currentLight, intersect);

			// add specular lighting
			output += applySpecular(lightRay, currentLight, lightProjection, viewRay, intersect);
		}
    }

	return output;
}

