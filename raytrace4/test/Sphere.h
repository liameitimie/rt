#pragma once
#include "cuda_runtime.h"
#include "Ray.h"
#include "myMath.h"

struct Sphere{
	float r;     //半径 
	float3 p,e,c;//球心位置，自发光，颜色 
	float roughness;
};

__host__ __device__
float intersect(const Sphere& s,const Ray& r,float tmin,float tmax){
//	解方程：(d.d)*t^2 + 2*((o-p).d)*t + (o-p).(o-p)-R^2 = 0
	float3 op=s.p-r.o;
	float b=dot(op,r.d),det=b*b-dot(op,op)+s.r*s.r;
	if(det<0) return 0;//delta小于0没有解
	det=sqrt(det);
	float t;
	t=b-det;
	if(t>=tmin&&t<tmax) return t;
	t=b+det;
	if(t>=tmin&&t<tmax) return t;
	return 0;
}
