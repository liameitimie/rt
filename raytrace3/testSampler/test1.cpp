#include <iostream>
#include <cuda_runtime.h>
#include <opencv2/opencv.hpp>
#include "../myMath.h"
#include "../Sobol.h"

#define WIDTH 1024
#define HEIGHT 1024
#define PI 3.141592653589793f

using namespace std;

float3 c[1024*1024];

//float a;
//float2 n,v,r,h;

//float randf(){
//	return (float)rand()/RAND_MAX;
//}

void setPixel(int x,int y,float3 color){
	if(x<0||x>=WIDTH||y<0||y>=HEIGHT) return;
//	cout<<x<<' '<<y<<' '<<(HEIGHT-y-1)*WIDTH+x<<endl;
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

float2 t[100005];

bool cmp(float2 a,float2 b){
	return a.x>b.x;
}

int main(){
	float3 write={255.0f,255.0f,255.0f};
	float3 red={255.0f,179.0f,50.0f};
	
	for(int n=1;n<=100000;n++){
		
		float x1=sobol(1,n);
		
		float a=1;
		float y=sqrt((1-x1)/((a*a-1)*x1+1));
		float x=sqrt(1-y*y);
		
//		float y=x1;
//		float x=sqrt(1-y*y);
		
//		float y=sqrt(1-x1);
//		float x=sqrt(x1);
		
		t[n]={x,y};
		
		if(n%10==0){
			clear();
			sort(t+1,t+n+1,cmp);
			float2 lst;
			for(int i=0;i<=89;i++){
				
				float bias=PI/360;
				float cb=cos(bias);
				
				float alp=(i+0.5)*PI/180;
				float2 tp={cos(alp),sin(alp)};
				
				int cnt=0;
				int j=lower_bound(t+1,t+n+1,tp,cmp)-t;
				
				for(int k=j;k<=n;k++){
					if(dot(tp,t[k])>cb) cnt++;
					else break;
				}
				for(int k=j-1;k>=1;k--){
					if(dot(tp,t[k])>cb) cnt++;
					else break;
				}
				
				float area=(2*bias)*(2*PI*abs(tp.x));
				float percent=(float)cnt/n/area/2;
				
				float scal=1;
				float2 cur=tp*percent*scal;
				
				drawLine(make_float2(0,0),cur,write);
				drawLine(make_float2(0,0),make_float2(-cur.x,cur.y),write);
//				if(i){
//					drawLine(lst,cur,write);
//					drawLine(make_float2(-lst.x,lst.y),make_float2(-cur.x,cur.y),write);
//				}
//				lst=cur;
//				if(i==89){
//					drawLine(make_float2(-lst.x,lst.y),make_float2(cur.x,cur.y),write);
//				}
			}
			cv::Mat image(WIDTH,HEIGHT,CV_32FC3,&c[0]);
			image.convertTo(image, CV_8UC3,1.0f);
			cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
			cv::imshow("image",image);
			cv::waitKey(1);
		}
	}
	
//	n={0,1};
//	v=normalize(float2{-1.5,1});
//	a=0.01;
	
//	drawLine(n,write);
//	drawLine(v,write);
	
//	for(int ti=1;ti<=1000;ti++){
////		float x1=randf();
////		float y=sqrt((1-x1)/((a*a-1)*x1+1));
////		float x=sqrt(1-y*y);
////		if(rand()%2) x=-x;
////		
////		h={x,y};
////		
////		r=(2*dot(h,v))*h-v;
////		drawLine(h,red);
////		drawLine(r,write);
//		
////		float x1=randf();
////		float y=x1;
////		float x=sqrt(1-y*y);
////		if(rand()%2) x=-x;
//		
//		float x1=randf();
//		float y=sqrt(1-x1);
//		float x=sqrt(x1);
//		if(rand()%2) x=-x;
//		
//		r={x,y};
//		drawLine(r,write);
//		
//		cv::Mat image(WIDTH,HEIGHT,CV_32FC3,&c[0]);
//		image.convertTo(image, CV_8UC3,1.0f);
//		cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
//		cv::imshow("image",image);
//		cv::waitKey(1);
//	}
	return 0;
}
/*
nvcc -I C:\opencv\build\include -I C:\opencv\build\include\opencv2 -L C:\opencv\build\x64\vc15\lib -l opencv_world454 -o test.exe test.cpp
*/
