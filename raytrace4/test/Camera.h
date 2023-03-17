#pragma once

#include "cuda_runtime.h"
#include "Ray.h"
#include "myMath.h"

struct Camera{
	float3 position;
	float3 front;
	float3 up;
	float3 right;
	float3 worldUp;
	
	float yaw;
	float pitch;
	
	float fov;
	
	__host__ __device__
	Ray genRay(float x,float y,int width,int height){
		float dp=2*tan(fov/2.0f)/height;
		float3 dx=((x-width*0.5f)*dp)*right;
		float3 dy=((y-height*0.5f)*dp)*up;
		float3 dir=normalize(front+dx+dy);
		return Ray(position,dir);
	}
};
