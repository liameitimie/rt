#pragma once

#include "Sobol.h"

class Sampler
{
public:

	Sampler(uint x, uint y, uint SampleIndex)
	{
		uint seed = ((x * 1973 + y * 9277) + 114514) | 1;
		Offset[0] = rhash(seed) * (1.0f / 4294967296.0f);
		Offset[1] = rhash(seed) * (1.0f / 4294967296.0f);

		Dimension = 0;
		Index = SampleIndex;
	}

	float Next1D()
	{
		float xi = Sobol::Sobol(Index, Dimension++) + Offset[0];
		if (xi > 1.0f) xi -= 1.0f;

		return xi;
	}

	//float2 Next2D()
	//{
	//	float xi1 = Sobol::Sobol(Index, Dimension++) + Offset[0];
	//	float xi2 = Sobol::Sobol(Index, Dimension++) + Offset[1];
	//	if (xi1 > 1.0f) xi1 -= 1.0f;
	//	if (xi2 > 1.0f) xi2 -= 1.0f;

	//	return float2(xi1, xi2);
	//}

private:

	inline uint rhash(uint& seed) {
		seed = seed ^ 61 ^ (seed >> 16);
		seed *= 9;
		seed = seed ^ (seed >> 4);
		seed *= 0x27d4eb2d;
		seed = seed ^ (seed >> 15);
		return seed;
	}

	float Offset[2];
	uint Dimension;
	uint Index;
};
