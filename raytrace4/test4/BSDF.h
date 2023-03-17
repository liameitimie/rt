#pragma once
#include <cuda_runtime.h>
#include "../common/myMath.h"
#include "../common/Matrial.h"

__device__
inline float sqr(float x){return x*x;}
__device__
inline float Pow5(float x){
	float x2=x*x;
	return x2*x2*x;
}

__device__
inline float3 F_Schlick(float3 F0,float VoH){
	return F0+(1-F0)*Pow5(1-VoH);
}

__device__
inline float F_Fresnel(float IOR,float VoH){
	float cosi=VoH;
	float sini=sqrt(1-cosi*cosi);
	float sint=(1/IOR)*sini;
	if(sint>=1) return 1;
	float cost=sqrt(1-sint*sint);
	float rl=(IOR*cosi-cost)/(IOR*cosi+cost);
	float rp=(cosi-IOR*cost)/(cosi+IOR*cost);
	return 0.5*(rl*rl+rp*rp);
}

/*
  余弦重要性采样 (漫反射重要性采样) 
 */
__device__
inline float3 SampleCosH(float xi_1,float xi_2){
	float r=sqrt(xi_1),z=sqrt(1-xi_1),p=2*PI*xi_2;
	float x=r*cos(p),y=r*sin(p);
	return float3{x,y,z};
}
/*
  ggx重要性采样 (计算ggx函数的边缘概率密度，再按普通一维的重要性采样处理)
 */
__device__ 
inline float3 SampleGGX(float a,float xi_1,float xi_2){
	float z=sqrt((1-xi_1)/((a*a-1)*xi_1+1));
	float r=sqrt(1-z*z),p=2*PI*xi_2;
	float x=r*cos(p),y=r*sin(p);
	return float3{x,y,z};
}
/*
  ggx可见法线的重要性采样(ggx分布为半椭球的法线分布，
  将半椭球缩放为半球，均匀采样半球对于v的投影面，再转回半椭球) 
*/
__device__
float3 SampleGGXVNDF(float3 V,float a,float xi_1,float xi_2){
	float3 N=normalize(float3{V.x*a,V.y*a,V.z});
	float len2=N.x*N.x+N.y*N.y;
	float3 T=len2>0?float3{-N.y,N.x,0}/sqrt(len2):float3{1,0,0};
	float3 B=cross(N,T);
	float r=sqrt(xi_1),p=2*PI*xi_2;
	float x=r*cos(p),y=r*sin(p);
	float s=0.5*(1+N.z);
	y=(1-s)*sqrt(1-x*x)+s*y;
	float3 H=x*T+y*B+sqrt(1-x*x-y*y)*N;
	H=normalize(float3{a*H.x,a*H.y,H.z});
	return H;
}

__device__
float lambda_GGX(float a,float NoV){
	float a2=a*a;
	return 0.5*(sqrt(a2+(1-a2)*NoV*NoV)/NoV-1);
}

__device__
inline float3 refract(float3 v,float3 n,float IOR){
	float cosi=dot(n,v);
	if(cosi<=0) return float3{0,0,0};
	float sin2i=1-cosi*cosi;
	float sin2t=sqr(1/IOR)*sin2i;
	if(sin2t>=1) return float3{0,0,0};
	float cost=sqrt(1-sin2t);
	return (-1/IOR)*v+((1/IOR)*cosi-cost)*n;
}

struct BSDF{
	Matrial m;
	float3 P,T,B,N;
	float3 L,V,H;
	bool outside;
	
	__device__
	float3 WorldToLocal(float3 v){
		return float3{dot(v,T),dot(v,B),dot(v,N)};
	}
	__device__
	float3 LocalToWorld(float3 v){
		float x=T.x*v.x+B.x*v.y+N.x*v.z;
		float y=T.y*v.x+B.y*v.y+N.y*v.z;
		float z=T.z*v.x+B.z*v.y+N.z*v.z;
		return float3{x,y,z};
	}
	
	__device__
	void init(float3 p,float3 n,float3 v,Matrial& mt){
		m=mt;
		P=p,N=n,outside=true;
		if(dot(n,v)<0){
			N=-N,outside=false;
			m.IOR=1/m.IOR;
		}
		T=abs(N.x)>0.99?float3{0,1,0}:float3{1,0,0};
		B=normalize(cross(N,T));
		T=normalize(cross(N,B));
		V=WorldToLocal(v);
	}
	
	__device__
	float3 SampleBSDF(float xi_1,float xi_2,float xi_3){
		float a=sqr(m.roughness);
		H=SampleGGXVNDF(V,a,xi_1,xi_2);
		float VoH=dot(V,H);
		float F_Dielectric=F_Fresnel(m.IOR,VoH);
		
		float p_r=lerp(F_Dielectric,1,m.metallic);
		float p_t=(1-p_r)*m.specTrans;
		float p_d=1-p_r-p_t;
		
		if(xi_3<p_r){
			L=reflect(-V,H);
		}
		else if(xi_3>=p_r&&xi_3<p_r+p_t){
			L=refract(V,H,m.IOR);
		}
		else{
			L=SampleCosH(xi_1,xi_2);
			H=normalize(L+V);
		}
		
		float NoL=L.z;
		if((xi_3<p_r&&NoL<=0)||(xi_3>=p_r&&xi_3<p_r+p_t&&NoL>=0))
			return float3{0,0,0};
		
		NoL=abs(NoL);
		float NoV=V.z;
		
		float R0=sqr(1-m.IOR)/sqr(1+m.IOR);
		float3 F0=lerp(R0,m.baseColor,m.metallic);
		float3 F=lerp(F_Dielectric,F_Schlick(F0,VoH),m.metallic);
		
		if(xi_3<p_r+p_t){
			float GV=lambda_GGX(a,NoV);
			float GL=lambda_GGX(a,NoL);
			float g2_g1=(1+GV)/(1+GL+GV);
			if(xi_3<p_r) return F*(g2_g1/p_r);
			else return sqrt(m.baseColor)*(1-F)*(m.specTrans*g2_g1/(m.IOR*m.IOR*p_t));
		}
		else{
			return (1-F)*m.baseColor*((1-m.specTrans)/p_d);
		}
	}
	
	__device__
	float3 getSampleDir(){
		return LocalToWorld(L);
	}
};
//
//struct BSDFContext{
//	float3 baseColor;
//	float metallic;
//	float roughness;
//	float IOR;
//	float specularTint;
//	float sheen;
//	float sheenTint;
//	float clearcoat;
//	float clearcoatGloss;
//	float specTrans;
//	float3 P,T,B,N;
//	float3 L,V,H;
//	float NoV,NoL,NoH,VoH,LoH;
//	bool outside;
//};
//
//float3 WorldToLocal(float3 v,BSDFContext& c){
//	return float3{dot(v,c.T),dot(v,c.B),dot(v,c.N)};
//}
//float3 LocalToWorld(float3 v,BSDFContext& c){
//	return float3{c.T.x}
//}
//
//__device__
//void InitBSDFContext(BSDFContext& c,float3 P,float3 N,float3 V,Matrial& mt){
//	float NoV=dot(N,V);
//	bool outside=true;
//	if(NoV<0) NoV=-NoV,N=-N,outside=false;
//	c.P=P,c.N=N;
//	c.T=abs(c.N.x)>0.99?make_float3(0,1,0):make_float3(1,0,0);
//	c.T=normalize(cross(c.N,c.T));
//	c.B=normalize(cross(c.N,c.T));
//	c.NoV=NoV,c.outside=outside;
//	//复读 
//	c.baseColor=mt.baseColor;
//	c.metallic=mt.metallic;
//	c.roughness=mt.roughness;
//	c.IOR=outside?mt.IOR:1/mt.IOR;
//	c.specularTint=mt.specularTint;
//	c.sheen=mt.sheen;
//	c.sheenTint=mt.sheenTint;
//	c.clearcoat=mt.clearcoat;
//	c.clearcoatGloss=mt.clearcoatGloss;
//	c.specTrans=mt.specTrans;
//}
//
//
//__device__
//float3 SampleGGXVNDF(float3 V,float a,float xi_1,float xi_2){
//	float3 N=normalize(float3{V.x*a,V.y*a,V.z});
//	float len2=N.x*N.x+N.y*N.y;
//	float3 T=len2>0?float3{-N.y,N.x,0}/sqrt(len2):float3{1,0,0};
//	float3 B=cross(N,T);
//	float r=sqrt(xi_1),p=2*PI*xi_2;
//	float x=r*cos(p),y=r*sin(p);
//	float s=0.5*(1+N.z);
//	y=(1-s)*sqrt(1-x*x)+s*y;
//	float3 H=x*T+y*B+sqrt(1-x*x-y*y)*N;
//	H=normalize(float3{a*H.x,a*H.y,H.z});
//	return H;
//}
//
//
//__device__
//float3 SampleBSDF(BSDFContext& c,float xi_1,float xi_2,float xi_3){
//	
//}
