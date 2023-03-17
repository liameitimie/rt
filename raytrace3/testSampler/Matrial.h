#pragma once
#include <cuda_runtime.h>

struct Matrial{
	float3 baseColor;
	float metallic;
	float roughness;
	
	float3 emission;
};
