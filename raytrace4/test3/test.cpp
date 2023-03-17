#include <iostream>
#include <cuda_runtime.h>
#include <opencv2/opencv.hpp>
#include "../common/myMath.h"
#include "../common/Sobol.h"
#define WIDTH 1024
#define HEIGHT 1024

using namespace std;

float3 c[1024*1024];

void setPixel(int x,int y,float3 color){
	if(x<0||x>=WIDTH||y<0||y>=HEIGHT) return;
	c[(HEIGHT-y-1)*WIDTH+x]=color;
}
void drawLine(float2 s,float2 t,float3 color){
	int x0=(s.x*800+WIDTH)/2,y0=(s.y*800+HEIGHT)/2;
	int x1=(t.x*800+WIDTH)/2,y1=(t.y*800+HEIGHT)/2;
	int dx=abs(x1-x0),sx=x0<x1?1:-1;
	int dy=abs(y1-y0),sy=y0<y1?1:-1;
	int err=(dx>dy?dx:-dy)/2,e2;
	while(1){
		setPixel(x0,y0,color);
		if(x0==x1&&y0==y1) break;
		e2=err;
		if(e2>-dx) err-=dy,x0+=sx;
		if(e2< dy) err+=dx,y0+=sy;
	}
}
void drawPoint(float2 p,float3 color){
	int x=(p.x*800+WIDTH)/2,y=(p.y*800+HEIGHT)/2;
	setPixel(x,y,color);
}
void clear(){
	memset(c,0,sizeof(c));
}

float3 sampleGGXVNDF(float3 V,float a,float xi_1,float xi_2){
	V=normalize(float3{V.x*a,V.y*a,V.z});
	float len2=V.x*V.x+V.y*V.y;
	float3 X=len2>0?float3{-V.y,V.x,0}/sqrt(len2):float3{1,0,0};
	float3 Y=cross(V,X);
	float r=sqrt(xi_1),p=2*PI*xi_2;
	float x=r*cos(p),y=r*sin(p);
	float s=0.5*(1+V.z);
	y=(1-s)*sqrt(1-x*x)+s*y;
	float3 N=x*X+y*Y+sqrt(1-x*x-y*y)*V;
	N=normalize(float3{a*N.x,a*N.y,N.z});
	return N;
}

float a=0.2;
float3 V=normalize(float3{-20,0,1}),N={0,0,1};

int cnt[200],cnt1[200];

int main(){
	float3 write={255.0f,255.0f,255.0f};
	float3 red={255.0f,179.0f,50.0f};
	for(int n=1;n<=1000000;n++){
		float xi_1=sobol(1,n),xi_2=sobol(2,n);
		float3 H=sampleGGXVNDF(V,a,xi_1,xi_2);
		if(dot(H,float3{-1,0,0})>0) cnt[(int)(acos(dot(N,H))*180/PI)]++;
		else cnt1[(int)(acos(dot(N,H))*180/PI)]++;
		
//		cout<<(int)(acos(dot(N,H))*180/PI)<<endl;
		if(n%10) continue;
		
		clear();
		drawLine(float2{0,0},normalize(float2{-20,1}),write);
		drawLine(float2{-1,0},float2{1,0},write);
		
		for(int i=1;i<=179;i++){
			float scal=0.3;
			float alp=i*PI/180;
			float2 tp=float2{sin(alp),cos(alp)},tp1=float2{-tp.x,tp.y};
			float area=(PI/180)*(PI*abs(tp.x));
			
			float percent=(float)cnt[i]/n/area/2;
			float percent1=(float)cnt1[i]/n/area/2;
			
			tp*=percent1*scal,tp1*=percent*scal;
			
//			cout<<length(tp)<<' '<<length(tp1)<<endl;
			
			drawLine(float2{0,0},tp,red);
			drawLine(float2{0,0},tp1,red);
		}
		
//		float y=dot(N,H),x=sqrt(1-y*y);
//		if(dot(H,float3{-1,0,0})>0) x=-x;
//		
//		drawLine(float2{0,0},float2{x,y},red);
		
		cv::Mat image(WIDTH,HEIGHT,CV_32FC3,&c[0]);
		image.convertTo(image, CV_8UC3,1.0f);
		cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
		cv::imshow("image",image);
		cv::waitKey(1);
	}
	return 0;
}
/*
nvcc -I C:\opencv\build\include -I C:\opencv\build\include\opencv2 -L C:\opencv\build\x64\vc15\lib -l opencv_world454 -o test.exe test.cpp
*/
