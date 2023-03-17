#include <cuda_runtime.h>
#include "Sobol.h"
#include "myMath.h"

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
  为每个像素的每个随机数添加一个偏移量
  避免每个像素使用同样的随机数列而导致的失真 
  只需保证每个像素大致不同即可 
 */
__device__
inline void rOffset(float* dst,int n,uint x,uint y){
	uint seed=((x*1973+y*9277)+114514)|1;
	for(int i=0;i<n;i++){
		dst[i]=rhash(seed)/4294967296.0f;
	}
}

/*
  取sobol(低差异序列)作为随机数，并添加偏移量
  注意：对于同一次光线的跟踪应使用同一sobol向量的不同维度，
	不同的光线应使用不同的向量 
 */
__device__
inline void rSobol(float* dst,int n,int Dstart,int t,float* offset){
	for(int i=0;i<n;i++){
		float x=sobol(Dstart+i,t)+offset[i];
		if(x>=1) x-=1;
		dst[i]=x;
	}
}

