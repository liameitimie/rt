#include "cuda_runtime.h"
#include "device_launch_parameters.h"
//#include "curand.h"
//#include "curand_kernel.h"
#include "cudaChecker.h"

#include <iostream>
#include "myMath.h"
#include "Ray.h"
#include "Sphere.h"
#include "Camera.h"
#include "Matrial.h"
#include "Sampler.h"

using namespace std;

#define WIDTH 1024
#define HEIGHT 768
#define SAMPLE_PER_PIXCEL 1024
//#define PI 3.141592653589793f

Matrial light={make_float3(0), 0, 0, make_float3(12,12,12)};

Matrial nr_black={make_float3(0.1,0.1,0.1), 0, 1, make_float3(0)};
Matrial nr_write={make_float3(1.0,1.0,1.0), 0, 1, make_float3(0)};
Matrial nr_blue={make_float3(0.35,0.35,1.0), 0, 1, make_float3(0)};
Matrial nr_red={make_float3(1.0,0.35,0.35), 0, 1, make_float3(0)};

Matrial ms_blue={make_float3(0.35,0.35,1.0), 1, 0.2, make_float3(0)};
Matrial ns_red={make_float3(1.0,0.35,0.35), 0, 0, make_float3(0)};

Matrial mr_write={make_float3(1.0,1.0,1.0), 1, 0.2, make_float3(0)};
Matrial ms_write={make_float3(1.0,1.0,1.0), 1, 0, make_float3(0)};

Matrial mr1={make_float3(1.0,1.0,1.0), 0, 0, make_float3(0)};
Matrial mr2={make_float3(1.0,1.0,1.0), 0, 0.25, make_float3(0)};
Matrial mr3={make_float3(1.0,1.0,1.0), 0, 0.5, make_float3(0)};
Matrial mr4={make_float3(1.0,1.0,1.0), 0, 0.75, make_float3(0)};
Matrial mr5={make_float3(1.0,1.0,1.0), 0, 1, make_float3(0)};

Sphere* d_s;
//Sphere h_s[]={
//	Sphere(16.5,make_float3(27,16.5,47),mr1),
//	Sphere(16.5,make_float3(73,16.5,78),mr2),
//	Sphere(16.5,make_float3(27,55,47),mr3),
//	Sphere(16.5,make_float3(73,55,78),mr4),
//	Sphere(16.5,make_float3(27,93,47),mr5)
//};
Sphere h_s[]={
	//    半径  位置                           材质 
	Sphere(1e5, make_float3( 1e5+1,40.8,81.6), ms_blue),//左墙壁 
	Sphere(1e5, make_float3(-1e5+99,40.8,81.6),ns_red),//右墙壁 
	Sphere(1e5, make_float3(50,40.8, 1e5),     nr_write),//后墙壁 
	Sphere(1e5, make_float3(50,40.8,-1e5+250), nr_black),//前墙壁 
	Sphere(1e5, make_float3(50, 1e5, 81.6),    nr_write),//地板
	Sphere(1e5, make_float3(50,-1e5+81.6,81.6),nr_write),//天花板
	Sphere(16.5,make_float3(27,16.5,47),       mr_write),//镜面球 
	Sphere(16.5,make_float3(73,16.5,78),       nr_write),//漫反射球 
	Sphere(600, make_float3(50,681.6-.27,81.6),light) //光源
};

int n=sizeof(h_s)/sizeof(Sphere);

float4 h_c[HEIGHT][WIDTH];
float4* d_c;

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
void castRay(Sphere* d_s,int n,float4* d_c,Camera cam,int T){
	int x=blockIdx.x,y=blockIdx.y;
	int idx=x+y*WIDTH;
	
	float xi[3],offset[3];//随机变量与偏移量 
	rOffset(offset,3,x,y); 
	
	__shared__ float4 c[SAMPLE_PER_PIXCEL];
	int ti=threadIdx.x;
	
	rSobol(xi,2,1,T*SAMPLE_PER_PIXCEL+ti,offset);
	Ray r=cam.genRay(x+xi[0],y+xi[1],WIDTH,HEIGHT);
	float3 cl=make_float3(0.0f),cf=make_float3(1);
	int dep=0;
	while(1){
		float t;int id;
		if(!closestHit(r,d_s,n,t,id)){/*cl=make_float3(0.5);*/break;}
		Sphere obj=d_s[id];
		
		Matrial& mt=obj.matrial;
		cl+=cf*mt.emission;
		//cf*=obj.c;
		if(++dep>6) break;
		
		float3 p=r.o+r.d*t;
		float3 tn=normalize(p-obj.p);
		float3 n=dot(tn,r.d)<0?tn:-tn;
		p+=n*make_float3(1e-1);
		float3 v=-r.d;
		
		rSobol(xi,3,dep*3,T*SAMPLE_PER_PIXCEL+ti,offset);
		
//		float3 l=SpUniformH(xi_1,xi_2,n);
//		float pdf=PdfUniformH(l,n);
//		float3 l=SpCosH(xi[0],xi[1],n);
//		float pdf=PdfCosH(l,n);
		float3 l=SpBRDF(xi[0],xi[1],xi[2],v,n,mt.metallic,mt.roughness);
		float pdf=PdfBRDF(l,v,n,mt.metallic,mt.roughness);
		
		if(pdf<1e-4) break;
		
		float3 fr=BRDF(v,l,n,mt.baseColor,mt.metallic,mt.roughness);
//		float3 fr=BRDF(l,v,n,mt.baseColor,mt.metallic,mt.roughness);
		
		cf*=fr*dot(n,l)/pdf;
		
		r=Ray(p,l);
		
		
//		cl=cf*0.5;
//		break;
	}
//	if(cl.x<0||cl.y<0||cl.z<0) cl=make_float3(1);
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
	
	Camera cam;
	cam.position=make_float3(50,52,215.6);
	cam.front=make_float3(0,0,-1);
	cam.right=make_float3(1,0,0);
	cam.up=make_float3(0,1,0);
	cam.fov=PI/4;
	
for(int i=0;i<20;i++){

	castRay<<<dim3(WIDTH,HEIGHT),SAMPLE_PER_PIXCEL>>>(d_s,n,d_c,cam,i);
	CHECK_KERNEL();
}
	
	CHECK(cudaMemcpy(h_c,d_c,sizeof(h_c),cudaMemcpyDeviceToHost));
	
	//CHECK(cudaFree(states));
	CHECK(cudaFree(d_c));
	CHECK(cudaFree(d_s));
	
	//写入图片（ppm格式，直接将rgb值写入） 
	freopen("image1.ppm","w",stdout);
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
