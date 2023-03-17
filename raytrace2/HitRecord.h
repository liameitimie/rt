#pragma once
#include <cuda_runtime.h>
#include "Matrial.h"

struct HitRecord{
	float t;
	float3 p;
	float3 normal;
	Matrial matrial;
	float2 uv;
	bool frontFace;
};
