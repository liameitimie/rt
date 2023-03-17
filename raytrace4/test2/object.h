#pragma once
#include <cuda_runtime.h>
#include "myMath.h"
#include "Ray.h"
#include "Matrial.h"

struct object{
	float r;
	float3 p;
	float3 p1,p2,p3;
	enum object_type{
		triangle,sphere
	};
	object_type objectType;
	Matrial matrial;
};

__device__ __host__
float hit(Ray& r,object& obj,float tmin,float tmax){
	if(obj.objectType==object::sphere){
		//	解方程：(d.d)*t^2 + 2*((o-p).d)*t + (o-p).(o-p)-R^2 = 0
		float3 op=obj.p-r.o;
		float b=dot(op,r.d),det=b*b-dot(op,op)+obj.r*obj.r;
		if(det<=0) return 0;//delta小于0没有解
		det=sqrt(det);
		float t;
		t=b-det;
		if(t>=tmin&&t<tmax) return t;
		t=b+det;
		if(t>=tmin&&t<tmax) return t;
	}
	else{
		float3 e1=obj.p2-obj.p1;
		float3 e2=obj.p3-obj.p1;
		float3 s=r.o-obj.p1;
		float3 s1=cross(r.d,e2);
		float3 s2=cross(s,e1);
		float d=dot(s1,e1);
		if(abs(d)<1e-6) return 0;
		d=1/d;
		float t=dot(s2,e2)*d;
		float b1=dot(s1,s)*d;
		float b2=dot(s2,r.d)*d;
		if(t>=tmin&&t<tmax&&b1>=0&&b2>=0&&b1+b2<=1) return t;
	}
	return 0;
}

__device__ __host__
float3 CalcNormal(object& obj,float3 p){
	if(obj.objectType==object::sphere){
		return normalize(p-obj.p);
	} 
	else{
		float3 e1=obj.p2-obj.p1;
		float3 e2=obj.p3-obj.p1;
		return normalize(cross(e1,e2));
	}
}
