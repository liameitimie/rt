#include <cuda_runtime.h>
#include "BRDF.h" 
#include "Sobol.h"
#include "myMath.h"

typedef unsigned int uint;

__device__
inline uint rhash(uint& seed){
	seed=seed^61^(seed>>16);
	seed*=9;
	seed=seed^(seed>>4);
	seed*=0x27d4eb2d;
    seed=seed^(seed>>15);
    return seed;
}

/* 
  Ϊÿ�����ص�ÿ����������һ��ƫ����
  ����ÿ������ʹ��ͬ����������ж����µ�ʧ�� 
  ֻ�豣֤ÿ�����ش��²�ͬ���� 
 */
__device__
inline void rOffset(float* dst,int n,uint x,uint y){
	uint seed=((x*1973+y*9277)+114514)|1;
	for(int i=0;i<n;i++){
		dst[i]=rhash(seed)/4294967296.0f;
	}
}

/*
  ȡsobol(�Ͳ�������)��Ϊ������������ƫ����
  ע�⣺����ͬһ�ι��ߵĸ���Ӧʹ��ͬһsobol�����Ĳ�ͬά�ȣ�
	��ͬ�Ĺ���Ӧʹ�ò�ͬ������ 
 */
__device__
inline void rSobol(float* dst,int n,int Dstart,int t,float* offset){
	for(int i=0;i<n;i++){
		float x=sobol(Dstart+i,t)+offset[i];
		if(x>=1) x-=1;
		dst[i]=x;
	}
}

/*
  ������ͶӰ����nΪ���ߵİ��� (��λ�����) 
 */
__device__
inline float3 toNormalH(float3 v,float3 n){
	float3 tp=make_float3(1,0,0);
	if(abs(n.x)>0.99) tp=make_float3(0,1,0);
	float3 t=normalize(cross(n,tp));
	float3 b=normalize(cross(n,t));
	return v.x*t+v.y*b+v.z*n;
}

/*
  ������ͶӰ����nΪ���ߵİ��� (ָ������) 
 */
__device__
inline float3 toNormalH(float3 v,float3 n,float3 t){
	float3 b=normalize(cross(n,t));
	t=normalize(cross(n,b));
	return v.x*t+v.y*b+v.z*n;
}

/*
  ������Ȳ��� 
 */
__device__
inline float3 SpUniformH(float xi_1,float xi_2,float3 n){
	float r=sqrt(1-xi_1*xi_1),z=xi_1;
	float x=r*cos(2*PI*xi_2),y=r*sin(2*PI*xi_2);
	return toNormalH(make_float3(x,y,z),n);
}
__device__
inline float PdfUniformH(){
	return 1/(2*PI);
}

/*
  ����������Ҫ�Բ��� (��������Ҫ�Բ���) 
 */
__device__
inline float3 SpCosH(float xi_1,float xi_2,float3 n){
	float r=sqrt(xi_1),z=sqrt(1-xi_1);
	float x=r*cos(2*PI*xi_2),y=r*sin(2*PI*xi_2);
	return toNormalH(make_float3(x,y,z),n);
}
__device__
inline float PdfCosH(float3 l,float3 n){
	return dot(n,l)/PI;
}

/*
  ggx��Ҫ�Բ��� (����ggx�����ı�Ե�����ܶȣ��ٰ���ͨһά����Ҫ�Բ�������)
 */
__device__ 
inline float3 SpGGX(float xi_1,float xi_2,float3 v,float3 n,float a){
	float z=sqrt((1-xi_1)/((a*a-1)*xi_1+1));
	float r=sqrt(1-z*z);
	float x=r*cos(2*PI*xi_2),y=r*sin(2*PI*xi_2);
	float3 h=toNormalH(make_float3(x,y,z),n);
	return reflect(-v,h);
}
__device__
inline float PdfGGX(float3 l,float3 v,float3 n,float a){
	float3 h=normalize(l+v);
	float NoH=dot(n,h),LoH=dot(l,h);
	float D=D_GGX(a,NoH);
	return D*NoH/(LoH*4);
}

/*
  BRDF��Ҫ�Բ��� (���ݽ����Ⱦ�������diffuse����specular�������ȸ�ʱ������ռ�ȵ�)
 */
__device__
float3 SpBRDF(float xi_1,float xi_2,float xi_3,float3 v,float3 n,float metallic,float roughness)
{
	float a=fmaxf(0.001,roughness*roughness);
	float p=(1-metallic)/(2-metallic);
	if(xi_3<p) return SpCosH(xi_1,xi_2,n);
	else return SpGGX(xi_1,xi_2,v,n,a);
}
__device__
float PdfBRDF(float3 l,float3 v,float3 n,float metallic,float roughness){
	if(dot(n,l)<0||dot(n,v)<0) return 0;
	float a=fmaxf(0.001,roughness*roughness);
	float p=(1-metallic)/(2-metallic);
	return PdfCosH(l,n)*p+PdfGGX(l,v,n,a)*(1-p);
}

