#pragma once
#include "cuda_runtime.h"

struct Ray{
	float3 o,d;
	
	__host__ __device__
	Ray(float3 _o,float3 _d){
		o=_o,d=_d;
	}
};
