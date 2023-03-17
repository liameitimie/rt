#include <iostream>
#include <cuda_runtime.h>
#include "cudaChecker.h"
#include "myMath.h"
#include "Sobol.h"
#include "Sphere.h"
#include "Camera.h"
#include "Sampler.h"

using namespace std;

#define WIDTH 1024
#define HEIGHT 768
#define SAMPLE_PER_PIXCEL 1024

float4 h_c[HEIGHT][WIDTH];
float4* d_c;

Sphere* d_s;
Sphere h_s[]={
	//    半径  位置                           自发光          颜色                     材质 
	Sphere{1e5, make_float3( 1e5+1,40.8,81.6), make_float3(0), make_float3(1.0,0.35,0.35),1},//左墙壁 
	Sphere{1e5, make_float3(-1e5+99,40.8,81.6),make_float3(0), make_float3(0.35,0.35,1.0),1},//右墙壁 
	Sphere{1e5, make_float3(50,40.8, 1e5),     make_float3(0), make_float3(1.0,1.0,1.0),1},//后墙壁 
	Sphere{1e5, make_float3(50,40.8,-1e5+250), make_float3(0), make_float3(0),          1},//前墙壁 
	Sphere{1e5, make_float3(50, 1e5, 81.6),    make_float3(0), make_float3(1.0,1.0,1.0),1},//地板
	Sphere{1e5, make_float3(50,-1e5+81.6,81.6),make_float3(0), make_float3(1.0,1.0,1.0),1},//天花板
	Sphere{16.5,make_float3(27,16.5,47),       make_float3(0), make_float3(1.0,1.0,1.0),0},//球 
	Sphere{16.5,make_float3(73,16.5,78),       make_float3(0), make_float3(1.0,1.0,1.0),0},//球 
	Sphere{600, make_float3(50,681.6-.27,81.6),make_float3(12,12,12),   make_float3(0), 1} //光源
};
int n=sizeof(h_s)/sizeof(Sphere);


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

__device__
float3 RTMain(Ray r,Sphere* d_s,int n,int maxdep,int ti,float* xi,float* offset){
	float3 sL={0,0,0},sK={1,1,1};
	int dep=0;
	while(1){
		float t;int id;
		if(!closestHit(r,d_s,n,t,id)) break;
		Sphere& obj=d_s[id];
		
		sL+=sK*obj.e;
		if(++dep>maxdep) break;
		
		float a=obj.roughness*obj.roughness;
		rSobol(xi,2,dep*2,ti,offset);
		
		float3 p=r.o+r.d*t;
		float3 n=normalize(p-obj.p);
		float3 v=-r.d;
		float3 l;
		float IOR=1.5;
		if(dot(n,v)<=0) n=-n,IOR=-IOR;
		
		if(a>0.9){
			p+=n*0.1;
			l=SpCosH(xi[0],xi[1],n);
			//sK*=c/Pi*dot(n,l)/PdfCosH(l,n);
			sK*=obj.c;
		}
		else if(a<0.001){
			float F;
			
			float VoH=dot(v,n);
			float cosi=VoH;
			float sini=sqrt(1-cosi*cosi);
			float sint=1/IOR*sini;
			float cost;
			if(sint>=1) F=1;
			else{
				cost=sqrt(1-sint*sint);
				float rl=(IOR*cosi-cost)/(IOR*cosi+cost);
				float rp=(cosi-IOR*cost)/(cosi+IOR*cost);
				F=0.5*(rl*rl+rp*rp);
			}
			
			if(xi[0]<F){
				p+=n*0.1;
				l=(2*dot(n,v))*n-v;
			}
			else{
				p+=n*-0.1;
				l=-(1/IOR)*v+((1/IOR)*cosi-cost)*n;
			}
		}
		else{
			float3 h=SpGGX(xi[0],xi[1],n,a);
			
		}
		r.o=p,r.d=l;
	}
	return sL;
}

__global__
void RTKernel(Sphere* d_s,int n,float4* d_c,Camera cam){
	int x=blockIdx.x,y=blockIdx.y;
	int idx=x+y*WIDTH;
	__shared__ float4 c[SAMPLE_PER_PIXCEL];
	int ti=threadIdx.x;
	
	float xi[3],offset[3];
	rOffset(offset,3,x,y);
	rSobol(xi,2,1,233,offset);
	Ray r=cam.genRay(x+xi[0],y+xi[1],WIDTH,HEIGHT);
	
	float3 radiance=RTMain(r,d_s,n,12,ti,xi,offset);
	c[ti]=make_float4(radiance);
	__syncthreads();
	
	for(int i=SAMPLE_PER_PIXCEL/2;i>0;i/=2){
		if(ti<i) c[ti]+=c[ti+i];
		__syncthreads();
	}
	if(ti==0) d_c[idx]+=c[0]/SAMPLE_PER_PIXCEL;
}

int main(){
	CHECK(cudaMalloc(&d_c,sizeof(h_c)));
	CHECK(cudaMemset(d_c,0,sizeof(h_c)));
	CHECK(cudaMalloc(&d_s,sizeof(h_s)));
	CHECK(cudaMemcpy(d_s,h_s,sizeof(h_s),cudaMemcpyHostToDevice));
	
	Camera cam;
	cam.position=make_float3(50,52,215.6);
	cam.front=make_float3(0,0,-1);
	cam.right=make_float3(1,0,0);
	cam.up=make_float3(0,1,0);
	cam.fov=PI/4;
	
	RTKernel<<<dim3(WIDTH,HEIGHT),SAMPLE_PER_PIXCEL>>>(d_s,n,d_c,cam);
	CHECK_KERNEL();
	
	CHECK(cudaMemcpy(h_c,d_c,sizeof(h_c),cudaMemcpyDeviceToHost));
	
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
