#include <cuda_runtime.h>

struct RayDesc
{
	float3 Origin;
	float TMin;
	float3 Direction;
	float TMax;
}

struct rtAccelerationStructure;

extern "C"
void TraceRay(rtAccelerationStructure, RayDesc);
