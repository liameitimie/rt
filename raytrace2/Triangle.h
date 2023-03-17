#pragma once

#include <cuda_runtime.h>
#include "myMath.h"
#include "Vertex.h"
#include "Matrial.h"
#include "Ray.h"
#include "HitRecord.h"

struct Triangle{
	Vertex v0,v1,v2;
	Matrial matrial;
};

__device__ __host__
bool hit(Ray r,Triangle tr,float tmin,float tmax,HitRecord& rec){
	float3 e1=tr.v1.p-tr.v0.p;
	float3 e2=tr.v2.p-tr.v0.p;
	float3 s=r.o-tr.v0.p;
	float3 s1=cross(r.d,e2);
	float3 s2=cross(s,e1);
	float d=dot(s1,e1);
	if(abs(d)<1e-6) return false;
	d=1/d;
	float t=dot(s2,e2)*d;
	float b1=dot(s1,s)*d;
	float b2=dot(s2,r.d)*d;
	if(t>=tmin&&t<tmax&&b1>=0&&b2>=0&&b1+b2<=1){
		rec.t=t;
		rec.p=r.o+t*r.d;
		rec.matrial=tr.matrial;
		rec.uv=tr.v0.uv*(1-b1-b2)+tr.v1.uv*b1+tr.v2.uv*b2;
		float3 n=normalize(cross(e1,e2));
		rec.frontFace=dot(r.d,n)<0;
		rec.normal=rec.frontFace?n:-n;
		return true;
	}
	return false;
}
