#include <iostream>
#include <opencv2/opencv.hpp>
#include "cudaChecker.h"
using namespace std;
//using namespace cv;


int main(){
	//cv::Mat image = cv::imread("C:/BaiduNetdiskDownload/高清pbr贴图/大理石/Marble021/Marble021_4K_NormalDX.png", cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
	cv::Mat image = cv::imread("C:/BaiduNetdiskDownload/高清pbr贴图/大理石/Marble021/Marble021_4K_Roughness.png", cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
	//cv::Mat image=cv::imread("image.png"/*,IMREAD_ANYCOLOR|IMREAD_ANYDEPTH*/);
	//cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
	
	int w=image.cols;
	int h=image.rows;
	int imtype=image.type();
	int c=(imtype>>3)+1;//颜色通道数 (channel) 
	int t=(imtype&0x07);//opencv颜色格式 0:CV_8U, 1:CV8S, 2:CV_16U, 3:CV_16S, 4:CV_32S, 5:CV_32F, 6:CV_64F
	int b=(8<<(t>>1));  //每个颜色的位数 
	
//	cout<<w<<' '<<h<<' '<<imtype<<' '<<c<<' '<<t<<' '<<b<<endl;
//	cout<<image.isContinuous()<<endl;
	
	if(c==3){
		cv::cvtColor(image, image, cv::COLOR_RGB2RGBA);
		imtype=image.type();
		c=(imtype>>3)+1;
		t=(imtype&0x07);
		b=(8<<(t>>1));
	}
	uchar* pdata=image.data;
	
	cout<<w<<' '<<h<<' '<<imtype<<' '<<c<<' '<<t<<' '<<b<<endl;
	cout<<image.isContinuous()<<endl;
	
//	cout<<(int)pdata[0]<<' '<<(int)pdata[1]<<' '<<(int)pdata[2]<<endl;
	
	/*定义颜色通道格式*/
//	cudaChannelFormatDesc cfd=cudaCreateChannelDesc<float2>();
	//cudaChannelFormatDesc cfd=cudaCreateChannelDesc(8,8,8,0,cudaChannelFormatKindUnsigned);
//	cudaChannelFormatDesc cfd=cudaCreateChannelDesc(
//		(c>=1)?b:0, (c>=2)?b:0, (c>=3)?b:0, (c>=4)?b:0,
//		((t>4)? cudaChannelFormatKindFloat
//			:(((t&1)||t==4)? cudaChannelFormatKindSigned
//				:cudaChannelFormatKindUnsigned
//			)
//		)
//	);
//	cfd.x=(c>=1)?b:0,cfd.y=(c>=2)?b:0,cfd.z=(c>=3)?b:0,cfd.w=(c>=4)?b:0;
//	if(t>4) cfd.f=cudaChannelFormatKindFloat;
//	else{
//		if(t&1) cfd.f=cudaChannelFormatKindSigned;
//		else cfd.f=cudaChannelFormatKindUnsigned;
//	}
	
//	cout<<cfd.x<<' '<<cfd.y<<' '<<cfd.z<<' '<<cfd.w<<' '<<cfd.f<<endl;
//	
//	cudaArray_t a;
//	CHECK(cudaMallocArray(&a,&cfd,w,h));
	
//	cv::Vec3w cc=image.at<cv::Vec3w>(0,0);
//	cout<<(int)cc[0]<<' '<<(int)cc[1]<<' '<<(int)cc[2]<<endl;
	//imwrite("image.png",image);
//	int t=image.type();
//	cout<<t<<' '<<(t>>3)<<' '<<(t&0x07)<<endl;
	return 0;
}
/*
nvcc -L C:/opencv/build/x64/vc15/lib -l opencv_world454 -I C:/opencv/build/include -I C:/opencv/build/include/opencv2 -o test.exe test.cu
*/
