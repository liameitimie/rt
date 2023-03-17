#pragma once
#include <cuda_runtime.h>
#include "myMath.h"
#include "Matrial.h"

struct BSDFContext{
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
	float3 P,T,B,N;
	float3 L,V,H;
	float NoV,NoL,NoH,VoH,LoH;
	bool outside;
};

__device__
void InitBSDFContext(BSDFContext& c,float3 P,float3 N,float3 V,Matrial& mt){
	float NoV=dot(N,V);
	bool outside=true;
	if(NoV<0) NoV=-NoV,N=-N,outside=false;
	c.P=P,c.N=N;
	c.T=abs(c.N.x)>0.99?make_float3(0,1,0):make_float3(1,0,0);
	c.T=normalize(cross(c.N,c.T));
	c.B=normalize(cross(c.N,c.T));
	c.V=V,c.NoV=NoV,c.outside=outside;
	c.baseColor=mt.baseColor;
	c.metallic=mt.metallic;
	c.roughness=mt.roughness;
	c.IOR=outside?mt.IOR:1/mt.IOR;
	c.specularTint=mt.specularTint;
	c.specTrans=mt.specTrans;
}

__device__
inline float sqr(float x){return x*x;}
__device__
inline float Pow5(float x){
	float x2=x*x;
	return x2*x2*x;
}

__device__
float F_Fresnel(float IOR,float VoH){
	float cosi=VoH;
	float sini=sqrt(1-cosi*cosi);
	float sint=(1/IOR)*sini;
	if(sint>=1) return 1;
	float cost=sqrt(1-sint*sint);
	float rl=(IOR*cosi-cost)/(IOR*cosi+cost);
	float rp=(cosi-IOR*cost)/(cosi+IOR*cost);
	return 0.5*(rl*rl+rp*rp);
}

__device__
inline bool refract(float3 v,float3 n,float IOR,float3& l){
	float cosi=dot(n,v);
	if(cosi<=0) return false;
	float sin2i=1-cosi*cosi;
	float sin2t=sqr(1/IOR)*sin2i;
	if(sin2t>=1) return false;
	float cost=sqrt(1-sin2t);
	l=(-1/IOR)*v+((1/IOR)*cosi-cost)*n;
	return true;
}

/*
  半球余弦重要性采样 (漫反射重要性采样) 
 */
__device__
inline float3 SampleCosH(float xi_1,float xi_2){
	float r=sqrt(xi_1),z=sqrt(1-xi_1),p=2*PI*xi_2;
	float x=r*cos(p),y=r*sin(p);
	return make_float3(x,y,z);
}
/*
  ggx重要性采样 (计算ggx函数的边缘概率密度，再按普通一维的重要性采样处理)
 */
__device__ 
inline float3 SampleGGX(float xi_1,float xi_2,float a){
	float z=sqrt((1-xi_1)/((a*a-1)*xi_1+1));
	float r=sqrt(1-z*z),p=2*PI*xi_2;
	float x=r*cos(p),y=r*sin(p);
	return make_float3(x,y,z);
}

__device__ __host__
inline float V_SmithGGXCorrelated(float a,float NoL,float NoV){
	float a2=a*a;
	float GL=NoV*sqrt(a2+(1-a2)*NoL*NoL);
	float GV=NoL*sqrt(a2+(1-a2)*NoV*NoV);
	return 0.5/(GL+GV);
}
//V=G/(4*NoL*NoV)

__device__
float3 SampleBSDF(BSDFContext& c,float xi_1,float xi_2,float xi_3){
	if(c.NoV<=0) return make_float3(0);
	float p_r=lerp(F_Fresnel(c.IOR,c.NoV),1,c.metallic);
	float p_t=(1-p_r)*c.specTrans;
	float p_d=1-p_r-p_t;
	
	float a=sqr(c.roughness);
	if(xi_3<p_r+p_t){
		float3 v=SampleGGX(xi_1,xi_2,a);
		c.H=v.x*c.T+v.y*c.B+v.z*c.N;
		if(xi_3<p_r) c.L=reflect(-c.V,c.H);
		else{
			if(!refract(c.V,c.H,c.IOR,c.L))
				return make_float3(0);
		}
	}
	else{
		float3 v=SampleCosH(xi_1,xi_2);
		c.L=v.x*c.T+v.y*c.B+v.z*c.N;
		c.H=normalize(c.L+c.V);
	}
	
	
	c.NoH=dot(c.N,c.H);
	c.NoL=dot(c.N,c.L);
	if((xi_3<p_r&&c.NoL<=0)||(xi_3>=p_r&&xi_3<p_r+p_t&&c.NoL>=0))
		return make_float3(0);
	c.NoL=abs(c.NoL);
	c.VoH=dot(c.V,c.H);
	c.LoH=dot(c.L,c.H);
	
	float lum=dot(c.baseColor,make_float3(0.3,0.6,0.1));
	float3 Ctint=lum>0?(c.baseColor/lum):make_float3(1);
	
	float FH=Pow5(1-c.VoH);
	
	float3 F;
	{
		float R0=sqr(1-c.IOR)/sqr(1+c.IOR);
		float3 Cspec=lerp(make_float3(1),Ctint,c.specularTint);
		float3 F0=lerp(R0*Cspec,c.baseColor,c.metallic);
		float3 F_Schlick=lerp(F0,make_float3(1),FH);
		float F_Dielectric=F_Fresnel(c.IOR,c.VoH);
		F=lerp(make_float3(F_Dielectric),F_Schlick,c.metallic);
	}
	if(xi_3<p_r+p_t){
		float V=V_SmithGGXCorrelated(a,c.NoL,c.NoV);
		if(xi_3<p_r){
//			c.P+=0.01*c.N;
			float val=V*4*c.NoL*c.VoH/c.NoH;
			return F*(val/p_r);
		}
		else{
//			c.P-=0.01*c.N;
			float val=V*4*c.NoL*c.VoH/(c.IOR*c.IOR*c.NoH);
			return sqrt(c.baseColor)*(1-F)*(c.specTrans*val/p_t);
		}
	}
	else{
		float FV=Pow5(1-c.NoV);
		float FL=Pow5(1-c.NoL);
		float Rr=2*c.roughness*c.VoH*c.VoH;
		float diffuse=(1-0.5*FL)*(1-0.5*FV);
		float retro=Rr*(FL+FV+FL*FV*(Rr-1));
		return c.baseColor*((diffuse+retro)/p_d);
//		c.P+=0.01*c.N;
//		return ((1-F)*(1-c.specTrans)/p_d)*c.baseColor;
	}
	return make_float3(0);
}


