#pragma once
#include <cuda_runtime.h>

struct Matrial{
	float3 baseColor;
	float metallic;
	float roughness;
	float IOR;
	float specularTint;
	float sheen;
	float sheenTint;
	float clearcoat;
	float clearcoatGloss;
	float specTrans;
	
//	float3 emission;
};
