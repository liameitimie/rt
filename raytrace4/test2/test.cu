#include <iostream>
#include <cuda_runtime.h>
#include "cudaChecker.h"

#include "Sampler.h"
#include "object.h"
#include "BSDF.h"
#include "TextureManager.h"
#include "Camera.h"

using namespace std;

#define WIDTH 1024
#define HEIGHT 768
#define SAMPLE_PER_PIXCEL 512

float4 h_c[HEIGHT][WIDTH];
float4* d_c;

object* d_s;
object h_s[2];
int n=2;

void initScene(){
	h_s[0].objectType=object::sphere;
	h_s[0].p={0,1,0};
	h_s[0].r=1;
	h_s[0].matrial.baseColor={1.022,0.782,0.344};
	h_s[0].matrial.metallic=1;
	h_s[0].matrial.roughness=0.2;
	h_s[0].matrial.IOR=1.5;
	h_s[0].matrial.specTrans=0;
	
	h_s[1].objectType=object::triangle;
	h_s[1].p1={200,0,-100};
	h_s[1].p2={-100,0,-100};
	h_s[1].p3={-100,0,200};
	h_s[1].matrial.baseColor={0.5,0.5,0.5};
	h_s[1].matrial.roughness=1;
	h_s[1].matrial.IOR=1.5;
	h_s[1].matrial.specTrans=0;
}

__device__
bool closestHit(Ray r,object* d_s,int n,float& t,int& id){
	bool hitted=false;
	float tmin=1e-4,tmax=1e10;
	for(int i=0;i<n;i++){
		float tmp=hit(r,d_s[i],tmin,tmax);
		if(tmp>0){
			hitted=true,tmax=tmp;
			t=tmp,id=i;
		}
	}
	return hitted;
}

__global__
void RTKernel(object* d_s,int n,float4* d_c,Camera cam,cudaTextureObject_t tex){
	int x=blockIdx.x,y=blockIdx.y;
	int idx=x+y*WIDTH;
	__shared__ float4 c[SAMPLE_PER_PIXCEL];
	int ti=threadIdx.x;
	
	float xi[3],offset[3];
	rOffset(offset,3,x,y);
	
	float3 color={0,0,0};
	
for(int i=1;i<=20;i++){

	rSobol(xi,2,1,i*SAMPLE_PER_PIXCEL+ti,offset);
	Ray r=cam.genRay(x+xi[0],y+xi[1],WIDTH,HEIGHT);
	
	float3 sL={0,0,0},sK={1,1,1};
	int dep=0;
	while(1){
		float t;int id;
		if(!closestHit(r,d_s,n,t,id)){
			float v=acos(r.d.y)/PI;
			float u=0;
			if(v>0) u=atan2(r.d.z,r.d.x)/(2*PI)+0.2;
			float4 c=tex2D<float4>(tex,u,v);
			sL+=sK*make_float3(c);
			break;
		}
		object& obj=d_s[id];
		
		if(++dep>12) break;
		
		float3 p=r.o+r.d*t;
		float3 n=CalcNormal(obj,p);
		float3 v=-r.d;
		
//		sL=n*0.5+0.5;break;
		
		rSobol(xi,3,dep*3,i*SAMPLE_PER_PIXCEL+ti+1,offset);
		
		BSDFContext c;
		InitBSDFContext(c,p,n,v,obj.matrial);
		float3 K=SampleBSDF(c,xi[0],xi[1],xi[2]);
		
		sK*=K;
		if(length(sK)==0) break;
		
		r.o=c.P,r.d=c.L;
	}
	if(length(sK)<1e3) color+=sL/20;
}
	c[ti]=make_float4(color);
	__syncthreads();
	
	for(int i=SAMPLE_PER_PIXCEL/2;i>0;i/=2){
		if(ti<i) c[ti]+=c[ti+i];
		__syncthreads();
	}
	if(ti==0) d_c[idx]+=c[0]/SAMPLE_PER_PIXCEL;
}

float3 ACESFilm(float3 x){
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e),0.0f,1.0f);
}

int main(){
/*	for(float a=0;a<1;a+=0.1){
		printf("a:%f\n",a);
		for(float NoL=0;NoL<1;NoL+=0.1){
			for(float NoV=0;NoV<1;NoV+=0.1){
				printf("%f ",V_SmithGGXCorrelated(a,NoL,NoV));
			}
			printf("\n");
		}
	}*/
	
	initScene();
	
	CHECK(cudaMalloc(&d_c,sizeof(h_c)));
	CHECK(cudaMemset(d_c,0,sizeof(h_c)));
	CHECK(cudaMalloc(&d_s,sizeof(h_s)));
	CHECK(cudaMemcpy(d_s,h_s,sizeof(h_s),cudaMemcpyHostToDevice));
	
	Camera cam;
	cam.position=make_float3(0,1.5,5);
	cam.front=make_float3(0,0,-1);
	cam.right=make_float3(1,0,0);
	cam.up=make_float3(0,1,0);
	cam.fov=PI/4;
	
	cudaTextureObject_t tex=textureManager.texture("C:/BaiduNetdiskDownload/高清pbr贴图/hdr环境光/lebombo_4k.hdr");
	
	RTKernel<<<dim3(WIDTH,HEIGHT),SAMPLE_PER_PIXCEL>>>(d_s,n,d_c,cam,tex);
	CHECK_KERNEL();
	
	CHECK(cudaMemcpy(h_c,d_c,sizeof(h_c),cudaMemcpyDeviceToHost));
	
	freopen("image.ppm","w",stdout);
	cout<<"P3\n"<<WIDTH<<' '<<HEIGHT<<' '<<255<<endl;
	for(int i=HEIGHT-1;i>=0;i--){
		for(int j=0;j<WIDTH;j++){
			float3 c=make_float3(h_c[i][j]);
			//c=ACESFilm(c);
			c=clamp(c,0,1);
			c=make_float3(pow(c.x,1/2.2),pow(c.y,1/2.2),pow(c.z,1/2.2));
			int3 tc=make_int3(c*255);
			cout<<tc.x<<' '<<tc.y<<' '<<tc.z<<' ';
		}
	}
	
	return 0;
}
/*
nvcc -L C:/opencv/build/x64/vc15/lib -l opencv_world454 -I C:/opencv/build/include -I C:/opencv/build/include/opencv2 -o test.exe test.cu
*/
