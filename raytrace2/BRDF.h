#pragma once

#include <cuda_runtime.h>
#include "myMath.h"

#define PI 3.14159265358979323846f

__device__ __host__
inline float Pow5(float x){
	float x2=x*x;
	return x2*x2*x;
	//return x*x*x*x*x;
}

__device__ __host__
inline float3 F_Schlick(float3 F0,float VoH){
	return F0+(1-F0)*Pow5(1-VoH);
}

__device__ __host__
inline float D_GGX(float a,float NoH){
	float a2=a*a;
	float d=NoH*NoH*(a2-1)+1;
	return a2/(PI*d*d);
}

__device__ __host__
inline float G_SchlicksmithGGX(float a,float NoL,float NoV){
	float k=(a+1)*(a+1)/8;
	float GL=NoL/(NoL*(1-k)+k);
	float GV=NoV/(NoV*(1-k)+k);
	return GL*GV;
}

__device__ __host__
inline float3 BRDF(float3 V,
					float3 L,
					float3 N,
					float3 baseColor,
					float metallic,
					float roughness)
{
	float3 H=normalize(V+L);
	float NoV=clamp(dot(N,V),0.0f,1.0f);
	float NoL=clamp(dot(N,L),0.0f,1.0f);
	float NoH=clamp(dot(N,H),0.0f,1.0f);
	float VoH=clamp(dot(V,H),0.0f,1.0f);
	
	if(roughness<0.05) roughness=0.05;
	float a=roughness*roughness;
	float3 F0=lerp(make_float3(0.04),baseColor,metallic);
	
	float D=D_GGX(a,NoH);
	float G=G_SchlicksmithGGX(a,NoL,NoV);
	float3 F=F_Schlick(F0,/*NoV*/VoH);
	
	float3 kd=(1-F)*(1-metallic);
	
	return kd*baseColor/PI + /*(1-kd)*/D*F*G/(4*NoL*NoV+0.001);
}
