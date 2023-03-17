#pragma once

#include <cuda_runtime.h>

struct Matrial{
	float3 baseColor;
	float metallic;
	float roughness;
	
	float3 emission;
	
//	cudaTextureObject_t tex_baseColor;
//	cudaTextureObject_t tex_metallic;
//	cudaTextureObject_t tex_roughness;
};
