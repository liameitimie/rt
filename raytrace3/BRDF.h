#pragma once

#include <cuda_runtime.h>
#include "myMath.h"

__device__ __host__
inline float Pow5(float x){
	float x2=x*x;
	return x2*x2*x;
	//return x*x*x*x*x;
}

__device__ __host__
inline float3 Diffuse_GGXApprox(float3 albedo,float a,float NoL,float NoV,float NoH,float VoH){
	float facing=VoH*VoH;//0.5+0.5*LoV=1/4*len(L+V)=VoH*VoH
	float rough=facing*(0.9-0.4*facing)/*(0.5+NoH)/NoH*/;
	float smooth=1.05*(1-Pow5(1-NoL))*(1-Pow5(1-NoV));
	float single=1/PI*lerp(smooth,rough,a);
	float multi=0.1159*a;
	float3 diffuse=albedo*(single+albedo*multi);
	return diffuse;
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
inline float V_SmithGGXCorrelated(float a,float NoL,float NoV){
	float a2=a*a;
	float GL=NoV*sqrt(a2+(1-a2)*NoL*NoL);
	float GV=NoL*sqrt(a2+(1-a2)*NoV*NoV);
	return 0.5/(GL+GV);
}
__device__ __host__
inline float V_SmithGGXCorrelatedApprox(float a,float NoL,float NoV){
	return 0.5/lerp(2*NoL*NoV,NoL+NoV,a);
}

__device__ __host__
float3 BRDF(float3 V,
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
	
	float a=fmaxf(0.001f,roughness*roughness);
	float3 F0=lerp(make_float3(0.04),baseColor,metallic);
	
	float D=D_GGX(a,NoH);
	float G=V_SmithGGXCorrelatedApprox(a,NoL,NoV);
	float3 F=F_Schlick(F0,VoH);
	
	float3 albedo=baseColor*(1-metallic);
	float3 diffuse=Diffuse_GGXApprox(albedo,a,NoL,NoV,NoH,VoH);
	
	return (1-F)*diffuse + F*D*G;
}
