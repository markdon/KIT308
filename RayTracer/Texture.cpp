/*  The following code is a VERY heavily modified from code originally sourced from:
	Ray tracing tutorial of http://www.codermind.com/articles/Raytracer-in-C++-Introduction-What-is-ray-tracing.html
	It is free to use for educational purpose and cannot be redistributed outside of the tutorial pages. */

#include "Texture.h"
#include <algorithm>

// read a pixel (using bilinear interpolation) using real coordinates of the range [0, 1]
Colour readTexture(const Texture& t, float u, float v) 
{
	int sizeU = t.width, sizeV = t.height;
	unsigned int* tab = t.data;

	// calculate the integer coordinates to read from 
	u = fabsf(u);
    v = fabsf(v);
    int umin = int(sizeU * u);
    int vmin = int(sizeV * v);
    int umax = int(sizeU * u) + 1;
    int vmax = int(sizeV * v) + 1;
    float ucoef = fabsf(sizeU * u - umin);
    float vcoef = fabsf(sizeV * v - vmin);
    
    // the texture coordinates are clamped to the range [0, 1]
	umin = std::min(std::max(umin, 0), sizeU - 1);
    umax = std::min(std::max(umax, 0), sizeU - 1);
    vmin = std::min(std::max(vmin, 0), sizeV - 1);
    vmax = std::min(std::max(vmax, 0), sizeV - 1);

    // use bilinear interpolation along two coordinates u and v
	Colour outputColor = (1.0f - vcoef) * ((1.0f - ucoef) * Colour(tab[umin + sizeU * vmin]) + ucoef * Colour(tab[umax + sizeU * vmin]))
        + vcoef * ((1.0f - ucoef) * Colour(tab[umin + sizeU * vmax]) + ucoef * Colour(tab[umax + sizeU * vmax]));

	if (t.sRGB)
	{
		// We make sure the data that was in sRGB storage mode is brought back to a linear format. 
		// We don't need the full accuracy of the sRGBEncode function so a powf should be sufficient enough.
		outputColor.blue   = powf(outputColor.blue, 2.2f);
		outputColor.red    = powf(outputColor.red, 2.2f);
		outputColor.green  = powf(outputColor.green, 2.2f);
	}

	if (t.exposed)
	{
		// The LDR (low dynamic range) images were supposedly already exposed, but we need 
		// to make the inverse transformation so that we can expose them a second time.
		outputColor.blue  = -logf(1.001f - outputColor.blue);
		outputColor.red   = -logf(1.001f - outputColor.red);
		outputColor.green = -logf(1.001f - outputColor.green);
	}

	return outputColor;
}

// read a pixel (using bilinear interpolation) from the cubemap location pointed to by the ray direction
Colour readCubemap(const CubeMap& cm, const Vector& dir)
{	
	Colour outputColor(0.0f, 0.0f, 0.0f);

	if (!cm.textures[0].data) return outputColor;

	// work out which face of the cubemap the vector aims towards and then read from that texture
	if ((fabsf(dir.x) >= fabsf(dir.y)) && (fabsf(dir.x) >= fabsf(dir.z)))
    {
        if (dir.x > 0.0f)
        {
			outputColor = readTexture(cm.textures[CubeMap::RIGHT], 1.0f - (dir.z / dir.x + 1.0f) * 0.5f, (dir.y / dir.x+ 1.0f) * 0.5f);
        }
        else if (dir.x < 0.0f)
        {
			outputColor = readTexture(cm.textures[CubeMap::LEFT], 1.0f - (dir.z / dir.x + 1.0f) * 0.5f, 1.0f - (dir.y / dir.x + 1.0f) * 0.5f);
        }
    }
    else if ((fabsf(dir.y) >= fabsf(dir.x)) && (fabsf(dir.y) >= fabsf(dir.z)))
    {
        if (dir.y > 0.0f)
        {
            outputColor = readTexture(cm.textures[CubeMap::UP], (dir.x / dir.y + 1.0f) * 0.5f, 1.0f - (dir.z / dir.y + 1.0f) * 0.5f);
        }
        else if (dir.y < 0.0f)
        {
            outputColor = readTexture(cm.textures[CubeMap::DOWN], 1.0f - (dir.x / dir.y + 1.0f) * 0.5f, (dir.z / dir.y + 1.0f) * 0.5f);
        }
    }
    else if ((fabsf(dir.z) >= fabsf(dir.x)) && (fabsf(dir.z) >= fabsf(dir.y)))
    {
        if (dir.z > 0.0f)
        {
            outputColor = readTexture(cm.textures[CubeMap::FORWARD], (dir.x / dir.z + 1.0f) * 0.5f, (dir.y / dir.z + 1.0f) * 0.5f);
        }
        else if (dir.z < 0.0f)
        {
            outputColor = readTexture(cm.textures[CubeMap::BACKWARD], (dir.x / dir.z + 1.0f) * 0.5f, 1.0f - (dir.y / dir.z + 1.0f) * 0.5f);
        }
    }

	return outputColor;
}
