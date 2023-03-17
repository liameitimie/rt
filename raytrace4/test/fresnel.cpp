#include <iostream>
#include <cuda_runtime.h>
#include <opencv2/opencv.hpp>

#include "myMath.h"

#define WIDTH 1024
#define HEIGHT 1024
#define PI 3.141592653589793f

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
void drawLine(float2 v,float3 color){
	drawLine(float2{0,0},v,color);
}
void drawPoint(float2 p,float3 color){
	int x=(p.x*800+WIDTH)/2,y=(p.y*800+HEIGHT)/2;
	setPixel(x,y,color);
}
void clear(){
	memset(c,0,sizeof(c));
}


float F_Fresnel(float ni,float nt,float VoH){
	float cosi=VoH;
	if(cosi<=0){
		float t=ni; ni=nt,nt=t;
		cosi=-cosi;
	}
	float sini=sqrt(1-cosi*cosi);
	float sint=ni/nt*sini;
	if(sint>=1) return 1;
	float cost=sqrt(1-sint*sint);
	float rl=(nt*cosi-ni*cost)/(nt*cosi+ni*cost);
	float rp=(ni*cosi-nt*cost)/(ni*cosi+nt*cost);
	return 0.5*(rl*rl+rp*rp);
}

int main(){
	float3 write={255,255,255};
	float IOR=1.5;
	float2 v=normalize(float2{-1,-2});
	float2 n={0,1};
	float2 l1,l2;
	
	float F=0;
	
	float VoH=dot(v,n);
	float cosi=VoH;
	if(cosi<=0){IOR=1/IOR,cosi=-cosi;}
	float sini=sqrt(1-cosi*cosi);
	float sint=1/IOR*sini;
	float cost=0;
	if(sint>=1) F=1;
	else{
		cost=sqrt(1-sint*sint);
		float rl=(IOR*cosi-cost)/(IOR*cosi+cost);
		float rp=(cosi-IOR*cost)/(cosi+IOR*cost);
		F=0.5*(rl*rl+rp*rp);
	}
	l1=(2*dot(n,v))*n-v;
	l2=-(1/IOR)*v+((1/IOR)*cosi-cost)*n;
	
	drawLine(v,write),drawLine(n,write);
	/*drawLine(l1,write*F),*/drawLine(l2,write*(1-F));
	cout<<F<<' '<<l2.x<<' '<<l2.y<<endl;
	
/*	float3 write={255,255,255};
	float ni=1,nt=1.5;
	for(int i=0;i<=800;i++){
		float VoH=(float)i/800;
		float F=F_Fresnel(ni,nt,-VoH);
		
		float2 p={VoH*2-1,F*2-1};
		float2 p0={VoH*2-1,-1};
		drawPoint(p,write);
		drawPoint(p0,write);
	}*/
	cv::Mat image(WIDTH,HEIGHT,CV_32FC3,&c[0]);
	image.convertTo(image, CV_8UC3,1.0f);
	cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
	cv::imshow("image",image);
	cv::waitKey(0);
	return 0;
}
/*
nvcc -I C:\opencv\build\include -I C:\opencv\build\include\opencv2 -L C:\opencv\build\x64\vc15\lib -l opencv_world454 -o fresnel.exe fresnel.cpp
*/
