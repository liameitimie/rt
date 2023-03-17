#include <cuda_runtime.h>
#include "cudaChecker.h"
#include <iostream>
#include "OBJ_Loader.hpp"
#include "Triangle.h"
#include "HitRecord.h"
#include "Matrial.h"
#include "Sobol.h"
#include "BRDF.h"
#include "Camera.h"
#include <vector>
#include <string>

#define WIDTH 1024
#define HEIGHT 768
#define SAMPLE_PER_PIXCEL 256

using namespace std;


vector<Triangle> h_tris;
Triangle* d_tris;

void AddObj(string f,Matrial mt);
void initScene();


float4 h_c[HEIGHT][WIDTH];
float4* d_c;

__device__
uint rhash(uint& seed){
	seed=seed^61^(seed>>16);
	seed*=9;
	seed=seed^(seed>>4);
	seed*=0x27d4eb2d;
    seed=seed^(seed>>15);
    return seed;
}
__device__
float2 rOffset(uint x,uint y,uint ti){
	uint seed=((x*1973+y*9277)+114514)|1;
	float u=rhash(seed)/4294967296.0f;
	float v=rhash(seed)/4294967296.0f;
    return make_float2(u,v);
}
__device__
float rSobol(int d,int i,float offset){
	float x=sobol(d,i)+offset;
	if(x>=1) x--;
	return x;
}
__device__
float3 SampleHemisphere(float x1,float x2){//半球均匀采样 
	float z=x1;
	float r=sqrt(1-z*z);
	float phi=2*PI*x2;
	return make_float3(r*cos(phi),r*sin(phi),z);
}
__device__
float3 toNormalHemisphere(float3 v,float3 n){//将v投影到以n为法向的半球 
	float3 tp=make_float3(1,0,0);
	if(abs(n.x)>0.5) tp=make_float3(0,1,0);
	float3 t=normalize(cross(n,tp));
	float3 b=normalize(cross(n,t));
	return v.x*t+v.y*b+v.z*n;
}

__device__
bool closestHit(Ray r,Triangle* d_tris,int n,HitRecord& rec){
	bool hitted=false;
	float tmin=1e-4,tmax=1e10;
	for(int i=0;i<n;i++){
		bool t=hit(r,d_tris[i],tmin,tmax,rec);
		if(t) tmax=rec.t,hitted=true;
	}
	return hitted;
}

__global__
void kernel(Triangle* d_tris,int n,float4* d_c,Camera cam,uint T){
	int x=blockIdx.x,y=blockIdx.y;
	int idx=x+y*WIDTH;
	
	__shared__ float4 c[SAMPLE_PER_PIXCEL];
	int ti=threadIdx.x;
	float2 offset=rOffset(x,y,ti);
//	float2 offset=make_float2(0);
	
	int dep=0;
	float dx=rSobol(1,ti,offset.x),dy=rSobol(2,ti,offset.y);
	Ray r=cam.genRay(x+dx,y+dy,WIDTH,HEIGHT);
	float3 cl=make_float3(0.0f),cf=make_float3(1.0f);
	
	//printf("x:%d,y:%d,t:%d, dx:%f,dy:%f\n",x,y,ti,dx,dy);
	HitRecord rec;
	while(1){
		if(!closestHit(r,d_tris,n,rec)) break;
		
		Matrial& mt=rec.matrial;
		cl+=mt.emission*cf;
		if(++dep>6) break;
		
		float3 &p=rec.p;
		float3 &n=rec.normal;
		float3 v=-r.d;
		p+=n*0.1;
		
		float x1=rSobol(dep*2+0,T*1024+ti,offset.x);
		float x2=rSobol(dep*2+1,T*1024+ti,offset.y);
		//printf("x:%d,y:%d,t:%d, x1:%f,x2:%f\n",x,y,ti,x1,x2);
		float3 l=SampleHemisphere(x1,x2);
		l=toNormalHemisphere(l,n);
		
		float3 fr=BRDF(v,l,n,mt.baseColor,mt.metallic,mt.roughness);
		//float3 fr=mt.baseColor;
		//printf("%f, %f, %f\n",fr.x,fr.y,fr.z);
		cf*=fr*clamp(dot(n,l),0.0f,1.0f)*2*PI;
		r=Ray(p,l);
		
//		float3 H=normalize(n+l);
//		float3 F0=lerp(make_float3(0.04),mt.baseColor,mt.metallic);
//		cl=F_Schlick(F0,dot(v,n));
//		break;
	}
	c[ti]=make_float4(cl);
	__syncthreads();
	
	for(int i=SAMPLE_PER_PIXCEL/2;i>0;i/=2){
		if(ti<i) c[ti]+=c[ti+i];
		__syncthreads();
	}
	if(ti==0){
		d_c[idx]+=c[0]/(SAMPLE_PER_PIXCEL*40);
	}
}

int main(){
	CHECK(cudaMalloc(&d_c,sizeof(h_c)));
	CHECK(cudaMemset(d_c,0,sizeof(h_c)));
	
	initScene();
//	cout<<h_tris.size()<<endl;
	int tsz=sizeof(Triangle)*h_tris.size();
/*	HitRecord rec;
	int id=0;
	Ray r(make_float3(278,273,-800),make_float3(0,0,1));
	for(auto t:h_tris){
		printf("%d:",++id);
		if(hit(r,t,1e-4,1e20,rec)) printf("yes\n");
		printf("\n");
		printf("v0:(%f, %f, %f) ",t.v0.p.x,t.v0.p.y,t.v0.p.z);
		printf("v1:(%f, %f, %f) ",t.v1.p.x,t.v1.p.y,t.v1.p.z);
		printf("v2:(%f, %f, %f)\n",t.v2.p.x,t.v2.p.y,t.v2.p.z);
	}*/
	
	CHECK(cudaMalloc(&d_tris,tsz));
	CHECK(cudaMemcpy(d_tris,h_tris.data(),tsz,cudaMemcpyHostToDevice));
	
	Camera cam;
	cam.position=make_float3(278,273,-800);
	cam.front=make_float3(0,0,1);
	cam.right=make_float3(-1,0,0);
	cam.up=make_float3(0,1,0);
	cam.fov=PI/4;

for(int i=1;i<=40;i++){

	kernel<<<dim3(WIDTH,HEIGHT),SAMPLE_PER_PIXCEL>>>(d_tris,h_tris.size(),d_c,cam,i);
	CHECK_KERNEL();
}
	
	
	CHECK(cudaMemcpy(h_c,d_c,sizeof(h_c),cudaMemcpyDeviceToHost));
	
	//写入图片（ppm格式，直接将rgb值写入） 
	freopen("image2.ppm","w",stdout);
	cout<<"P3\n"<<WIDTH<<' '<<HEIGHT<<' '<<255<<endl;
	for(int i=HEIGHT-1;i>=0;i--){
		for(int j=0;j<WIDTH;j++){
			float4 c=h_c[i][j];
			c=clamp(c,0.0f,1.0f);
			//c=c/(c+1.0);
			c=make_float4(pow(c.x,1/2.2),pow(c.y,1/2.2),pow(c.z,1/2.2),1);
			int4 tc=make_int4(c*255);
			cout<<tc.x<<' '<<tc.y<<' '<<tc.z<<' ';
		}
	}
	
	return 0;
}


void AddObj(string f,Matrial mt){
	objl::Loader loader;
    loader.LoadFile(f);
    auto mesh=loader.LoadedMeshes[0];
    for(int i=0;i<mesh.Vertices.size();i+=3){
    	auto v0=mesh.Vertices[i+0].Position;
    	auto v1=mesh.Vertices[i+1].Position;
    	auto v2=mesh.Vertices[i+2].Position;
    	
    	Triangle tri;
    	tri.v0.p=make_float3(v0.X,v0.Y,v0.Z);
    	tri.v1.p=make_float3(v1.X,v1.Y,v1.Z);
    	tri.v2.p=make_float3(v2.X,v2.Y,v2.Z);
    	tri.matrial=mt;
    	
    	h_tris.push_back(tri);
	}
}

void initScene(){
	Matrial nr_write={make_float3(0.8),0,1,make_float3(0)};
	Matrial mr_write={make_float3(0.8),1,0.5,make_float3(0)};
	
	Matrial ns_write={make_float3(0.8),0,0.1,make_float3(0)};
	Matrial ms_write={make_float3(0.8),1,0.1,make_float3(0)};
	
	Matrial mr_blue={make_float3(0.35,0.35,1.0),1,1,make_float3(0)};
	Matrial ms_blue={make_float3(0.35,0.35,1.0),1,0.1,make_float3(0)};
	
	Matrial nr_blue={make_float3(0.35,0.35,1.0),0,1,make_float3(0)};
	Matrial ns_blue={make_float3(0.35,0.35,1.0),0,0.1,make_float3(0)};
	
	Matrial mr_red={make_float3(1.0,0.35,0.35),1,1,make_float3(0)};
	Matrial ms_red={make_float3(1.0,0.35,0.35),1,0.1,make_float3(0)};
	
	Matrial nr_red={make_float3(1.0,0.35,0.35),0,1,make_float3(0)};
	Matrial ns_red={make_float3(1.0,0.35,0.35),0,0.1,make_float3(0)};
	
	Matrial mr_yellow={make_float3(1.0,1.0,0.5),1,1,make_float3(0)};
	
	Matrial mr_ori={make_float3(1.0,0.5,0.3),1,1,make_float3(0)};
	
	Matrial light={make_float3(0),1,1,make_float3(30)};
	
	AddObj("cornellbox/floor.obj",mr_write);
	AddObj("cornellbox/shortbox.obj",mr_write);
	AddObj("cornellbox/tallbox.obj",mr_write);
	AddObj("cornellbox/left.obj",nr_red);
	AddObj("cornellbox/right.obj",nr_blue);
	AddObj("cornellbox/light.obj",light);
}
