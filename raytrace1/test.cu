#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "curand.h"
#include "curand_kernel.h"
#include "cudaChecker.h"

#include <iostream>
#include "myMath.h"
#include "Ray.h"
#include "Sphere.h"
#include "Camera.h"

using namespace std;

#define WIDTH 1024
#define HEIGHT 768
#define SAMPLE_PER_PIXCEL 1024
#define PI 3.141592653589793f

Sphere* d_s;
Sphere h_s[]={
	//    半径  位置                           自发光          颜色                     材质 
	Sphere(1e5, make_float3( 1e5+1,40.8,81.6), make_float3(0), make_float3(1.0,0.35,0.35),DIFF),//左墙壁 
	Sphere(1e5, make_float3(-1e5+99,40.8,81.6),make_float3(0), make_float3(0.35,0.35,1.0),DIFF),//右墙壁 
	Sphere(1e5, make_float3(50,40.8, 1e5),     make_float3(0), make_float3(1.0,1.0,1.0),DIFF),//后墙壁 
	Sphere(1e5, make_float3(50,40.8,-1e5+250), make_float3(0), make_float3(0),          DIFF),//前墙壁 
	Sphere(1e5, make_float3(50, 1e5, 81.6),    make_float3(0), make_float3(1.0,1.0,1.0),DIFF),//地板
	Sphere(1e5, make_float3(50,-1e5+81.6,81.6),make_float3(0), make_float3(1.0,1.0,1.0),DIFF),//天花板
	Sphere(16.5,make_float3(27,16.5,47),       make_float3(0), make_float3(1,1,1)*.999, SPEC),//镜面球 
	Sphere(16.5,make_float3(73,16.5,78),       make_float3(0), make_float3(1,1,1)*.999, DIFF),//漫反射球 
	Sphere(600, make_float3(50,681.6-.27,81.6),make_float3(12,12,12),   make_float3(0), DIFF) //光源
};

int n=sizeof(h_s)/sizeof(Sphere);

float4 h_c[HEIGHT][WIDTH];
float4* d_c;

float* d_samples;//采样点 

curandStateXORWOW_t* states;//随机数生成器状态
__global__
void init_curand(curandStateXORWOW_t* states,unsigned long long seed){
	int idx=threadIdx.x;
	curand_init(seed,idx,0,&states[idx]);
}


__device__
bool closestHit(const Ray& r,Sphere* d_s,int n,float& t,int& id){
	bool hitted=false;
	float tmin=1e-4,tmax=1e10;
	for(int i=0;i<n;i++){
		float tmp=intersect(d_s[i],r,tmin,tmax);
		if(tmp>0){
			hitted=true,tmax=tmp;
			t=tmp,id=i;
		}
	}
	return hitted;
}

__global__
void castRay(Sphere* d_s,int n,float4* d_c,Camera cam,float* d_samples,curandStateXORWOW_t* states){
	int x=blockIdx.x,y=blockIdx.y;
	int idx=x+y*WIDTH;
	
	__shared__ float4 c[SAMPLE_PER_PIXCEL];
	int ti=threadIdx.x;

	int dep=0;
	float dx=d_samples[ti];
	float dy=d_samples[2*SAMPLE_PER_PIXCEL-ti-1];
	Ray r=cam.genRay(x+dx,y+dy,WIDTH,HEIGHT);
	float3 cl=make_float3(0.0f),cf=make_float3(1.0f);
	
	while(1){
		float t;int id;
		if(!closestHit(r,d_s,n,t,id)) break;
		const Sphere& obj=d_s[id];
		
		cl+=cf*obj.e;
		cf*=obj.c;
		if(++dep>6) break;
		
		float3 p=r.o+r.d*t;
		float3 tn=normalize(p-obj.p);
		float3 n=dot(tn,r.d)<0?tn:-tn;
		p+=n*make_float3(1e-1);
		
		if(obj.refl==SPEC){
			float3 d=r.d-n*(2*dot(n,r.d));
			r=Ray(p,d);
		}
		else{
			float r1=2*PI*curand_uniform(&states[ti]);
			float r2=curand_uniform(&states[ti]);
			
			float3 u=normalize(cross(n,(abs(n.x)>0.5f?make_float3(0,1,0):make_float3(1,0,0))));
			float3 v=cross(u,n);
			float3 d=normalize((u*cos(r1)+v*sin(r1))*sqrt(r2)+n*sqrt(1-r2));
			r=Ray(p,d);
		}
	}
	c[ti]=make_float4(cl);
	__syncthreads();
	
	for(int i=SAMPLE_PER_PIXCEL/2;i>0;i/=2){
		if(ti<i) c[ti]+=c[ti+i];
		__syncthreads();
	}
	if(ti==0){
//		d_c[idx]+=c[0]/SAMPLE_PER_PIXCEL;
		d_c[idx]+=c[0]/(SAMPLE_PER_PIXCEL*20);
	}
}

int main(){
	CHECK(cudaSetDevice(0));
	
	//初始化gpu端帧缓冲 
	CHECK(cudaMalloc(&d_c,sizeof(h_c)));
	CHECK(cudaMemset(d_c,0,sizeof(h_c)));
	//初始化gpu端物体数据 
	CHECK(cudaMalloc(&d_s,sizeof(h_s)));
	CHECK(cudaMemcpy(d_s,h_s,sizeof(h_s),cudaMemcpyHostToDevice));
	//初始化gpu端随机数生成器的状态 
	CHECK(cudaMalloc(&states,sizeof(curandStateXORWOW_t)*SAMPLE_PER_PIXCEL));
	init_curand<<<1,SAMPLE_PER_PIXCEL>>>(states,time(0));
	
	//在cpu端生成采样点（均匀分布） 
	curandGenerator_t gen;
	checkCudaErrors(curandCreateGenerator(&gen, CURAND_RNG_PSEUDO_XORWOW));
	CHECK(cudaMalloc(&d_samples,sizeof(float)*SAMPLE_PER_PIXCEL*2));
	checkCudaErrors(curandGenerateUniform(gen,d_samples,SAMPLE_PER_PIXCEL*2));
	
	Camera cam;
	cam.position=make_float3(50,52,215.6);
	cam.front=make_float3(0,0,-1);
	cam.right=make_float3(1,0,0);
	cam.up=make_float3(0,1,0);
	cam.fov=PI/4;
	
//for(int i=0;i<20;i++){

	castRay<<<dim3(WIDTH,HEIGHT),SAMPLE_PER_PIXCEL>>>(d_s,n,d_c,cam,d_samples,states);
	CHECK_KERNEL();
//}
	
	CHECK(cudaMemcpy(h_c,d_c,sizeof(h_c),cudaMemcpyDeviceToHost));
	
	CHECK(cudaFree(states));
	CHECK(cudaFree(d_c));
	CHECK(cudaFree(d_s));
	
	//写入图片（ppm格式，直接将rgb值写入） 
	freopen("image.ppm","w",stdout);
	cout<<"P3\n"<<WIDTH<<' '<<HEIGHT<<' '<<255<<endl;
	for(int i=HEIGHT-1;i>=0;i--){
		for(int j=0;j<WIDTH;j++){
			float4 c=h_c[i][j];
			c=clamp(c,0.0f,1.0f);
			c=make_float4(pow(c.x,1/2.2),pow(c.y,1/2.2),pow(c.z,1/2.2),1);
			int4 tc=make_int4(c*make_float4(255));
			cout<<tc.x<<' '<<tc.y<<' '<<tc.z<<' ';
		}
	}
	return 0;
}
/*
nvcc -o test.exe -L "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.6/lib/x64" -l curand test.cu
*/
