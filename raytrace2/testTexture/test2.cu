#include <iostream>
#include <opencv2/opencv.hpp>
#include "cudaChecker.h"
#include "myMath.h"
using namespace std;
//using namespace cv;

#define WIDTH 4096
#define HEIGHT 2048

float4 h_c[HEIGHT][WIDTH];
float4* d_c;

__global__
void mycopy(float4* d_c,cudaTextureObject_t tex){
	int x=blockIdx.x,y=blockIdx.y;
	int idx=x+y*WIDTH;
	float u=(float)x/WIDTH,v=(float)y/HEIGHT;
	
	d_c[idx]=tex2D<float4>(tex,u,v);
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
	//cv::Mat image = cv::imread("C:/BaiduNetdiskDownload/高清pbr贴图/大理石/Marble021/Marble021_4K_NormalDX.png", cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
	//cv::Mat image = cv::imread("C:/BaiduNetdiskDownload/高清pbr贴图/大理石/Marble021/Marble021_4K_Roughness.png", cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
	//cv::Mat image=cv::imread("image2.png"/*,IMREAD_ANYCOLOR|IMREAD_ANYDEPTH*/);
	
	//cv::Mat image=cv::imread("C:/BaiduNetdiskDownload/高清pbr贴图/hdr环境光/lebombo_4k.hdr",cv::IMREAD_ANYCOLOR|cv::IMREAD_ANYDEPTH);
	//cv::Mat image=cv::imread("C:/BaiduNetdiskDownload/高清pbr贴图/hdr环境光/the_sky_is_on_fire_4k.hdr",cv::IMREAD_ANYCOLOR|cv::IMREAD_ANYDEPTH);
	cv::Mat image=cv::imread("C:/BaiduNetdiskDownload/高清pbr贴图/hdr环境光/studio_garden_4k.hdr",cv::IMREAD_ANYCOLOR|cv::IMREAD_ANYDEPTH);
	
	//cv::imwrite("image2.png",image);
	
	int w=image.cols;
	int h=image.rows;
	//if(!w&&!h) return 0; 
	int imtype=image.type();
	if((imtype>>3)+1==3){//3通道，cuda纹理不支持3通道
		cv::cvtColor(image,image,cv::COLOR_BGR2RGBA);
		imtype=image.type();
	}
	int c=(imtype>>3)+1;//颜色通道数 (channel) 
	int t=(imtype&0x07);//opencv颜色格式 0:CV_8U, 1:CV8S, 2:CV_16U, 3:CV_16S, 4:CV_32S, 5:CV_32F, 6:CV_64F
	int b=(8<<(t>>1));  //每个颜色通道的位数 
	
	int sz=w*h*c*(b>>3);
	uchar* pdata=image.data;
	
	cout<<image.size()<<' '<<sz<<endl;
	cout<<w<<' '<<h<<' '<<imtype<<' '<<c<<' '<<t<<' '<<b<<endl;
	cout<<image.isContinuous()<<endl;
	
	auto color=image.at<cv::Vec4f>(0,0);
	cout<<color[0]<<' '<<color[1]<<' '<<color[2]<<' '<<color[3]<<endl;
	
	for(int i=0;i<16;i+=4){
		float t;
		uchar* p=(uchar*)&t;
		for(int j=0;j<4;j++) p[j]=pdata[i+j];
		cout<<t<<endl;
		//cout<<(int)(pdata[0+i*1]/*+pdata[1+i*2]*256*/)<<' ';
	}cout<<endl;
	
	
	cudaChannelFormatDesc cfd=cudaCreateChannelDesc(
		(c>=1)?b:0, (c>=2)?b:0, (c>=3)?b:0, (c>=4)?b:0,
		((t>4)? cudaChannelFormatKindFloat
			:(((t&1)||t==4)? cudaChannelFormatKindSigned
				:cudaChannelFormatKindUnsigned
			)
		)
	);
	
	cout<<cfd.x<<' '<<cfd.y<<' '<<cfd.z<<' '<<cfd.w<<' '<<cfd.f<<endl;
	
	//创建cuda数组并复制图像到cuda数组 
	cudaArray_t a;
	CHECK(cudaMallocArray(&a,&cfd,w,h));
	CHECK(cudaMemcpyToArray(a,0,0,pdata,sz,cudaMemcpyHostToDevice));
	
	//设置资源描述 
	cudaResourceDesc rd;                //资源描述，描述资源的类型与资源地址 
    memset(&rd,0,sizeof(rd));
    rd.resType=cudaResourceTypeArray;   //资源类型为cuda数组， 
    rd.res.array.array=a;               //除此之外还有mipmap、linear、pitch2D 
    
    //设置纹理描述
	cudaTextureDesc td;                 //纹理描述，描述插值方法等纹理属性 
	memset(&td,0,sizeof(td));
	td.normalizedCoords=true;           //访问的uv坐标以归一化模式访问 
	td.filterMode=cudaFilterModeLinear; //插值方法为线性插值，0:最近邻, 1:线性 
	//td.readMode=cudaReadModeNormalizedFloat;//读取的数据归一化 
	td.readMode=cudaReadModeElementType;
	//td.addressMode[0..2] //当uv大于一时采取的策略，0:wrap, 1:clamp, 2:mirror, 3:border  
	
	//创建纹理对象 
	cudaTextureObject_t tex=0;
	CHECK(cudaCreateTextureObject(&tex,&rd,&td,NULL));
	
	CHECK(cudaMalloc(&d_c,sizeof(h_c)));
	
	mycopy<<<dim3(WIDTH,HEIGHT),1>>>(d_c,tex);
	
	CHECK(cudaMemcpy(h_c,d_c,sizeof(h_c),cudaMemcpyDeviceToHost));
	
	//写入图片（ppm格式，直接将rgb值写入） 
	freopen("image5.ppm","w",stdout);
	cout<<"P3\n"<<WIDTH<<' '<<HEIGHT<<' '<<255<<endl;
	for(int i=0;i<HEIGHT;i++){
		for(int j=0;j<WIDTH;j++){
			float3 c=make_float3(h_c[i][j]);
			c=ACESFilm(c);
			int r=c.x*255;
			int g=c.y*255;
			int b=c.z*255;
			cout<<r<<' '<<g<<' '<<b<<' ';
		}
	}
	
	return 0;
}
/*
nvcc -L C:/opencv/build/x64/vc15/lib -l opencv_world454 -I C:/opencv/build/include -I C:/opencv/build/include/opencv2 -o test2.exe test2.cu
*/
