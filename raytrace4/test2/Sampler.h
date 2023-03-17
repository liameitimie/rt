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

