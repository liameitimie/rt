#pragma once

#include <cuda_runtime.h>
#include "cudaChecker.h"
#include <opencv2/opencv.hpp>

#include <string>
#include <map>

using namespace std;

class TextureManager{
public:
	map<string,cudaTextureObject_t> mp;
	
	cudaTextureObject_t texture(string fname){
		if(!mp[fname]){
			cv::Mat image=cv::imread(fname,cv::IMREAD_ANYCOLOR|cv::IMREAD_ANYDEPTH);
			mp[fname]=createTexture(image);
		}
		return mp[fname];
	}
	
	cudaTextureObject_t createTexture(cv::Mat& image){
		int w=image.cols;
		int h=image.rows;
		
		if(!w&&!h) return 0;
		
		int imtype=image.type();
		if((imtype>>3)+1==3){//3通道，cuda纹理不支持3通道
			cv::cvtColor(image,image,cv::COLOR_BGR2RGBA);
			imtype=image.type();
		}
		int c=(imtype>>3)+1;//颜色通道数 (channel) 
		int t=(imtype&0x07);//opencv颜色格式 0:CV_8U, 1:CV8S, 2:CV_16U, 3:CV_16S, 4:CV_32S, 5:CV_32F, 6:CV_64F
		int b=(8<<(t>>1));  //每个颜色通道的位数 
		
		int sz=w*h*c*(b>>3); 
		uchar* p_data=image.data;
		
		/* 设置像素格式描述 */
		cudaChannelFormatDesc formatDesc=cudaCreateChannelDesc(
			(c>=1)?b:0, (c>=2)?b:0, (c>=3)?b:0, (c>=4)?b:0,
			((t>4)? cudaChannelFormatKindFloat
				:(((t&1)||t==4)? cudaChannelFormatKindSigned
					:cudaChannelFormatKindUnsigned
				)
			)
		);//根据opencv颜色格式特判生成cuda数组像素描述 
		
		/* 创建cuda数组并复制图像到cuda数组 */
		cudaArray_t array;
		CHECK(cudaMallocArray(&array,&formatDesc,w,h));
		CHECK(cudaMemcpyToArray(array,0,0,p_data,sz,cudaMemcpyHostToDevice));
		
		/* 设置资源描述 */
		cudaResourceDesc resDesc;                //资源描述，描述资源的类型与资源地址 
	    memset(&resDesc,0,sizeof(resDesc));
	    resDesc.resType=cudaResourceTypeArray;   //资源类型为cuda数组， 
	    resDesc.res.array.array=array;           //除此之外还有mipmap、linear、pitch2D 
	    
	    /* 设置纹理描述 */
		cudaTextureDesc texDesc;                 //纹理描述，描述插值方法等纹理属性 
		memset(&texDesc,0,sizeof(texDesc));
		texDesc.normalizedCoords=true;           //访问的uv坐标以归一化模式访问 
		texDesc.filterMode=cudaFilterModeLinear; //插值方法为线性插值，0:最近邻, 1:线性 
		texDesc.readMode=cudaReadModeNormalizedFloat;//读取的数据归一化 
		//td.addressMode[0..2] //当uv大于一时采取的策略，0:wrap, 1:clamp, 2:mirror, 3:border 
		
		/* 创建纹理对象 */
		cudaTextureObject_t tex=0;
		CHECK(cudaCreateTextureObject(&tex,&resDesc,&texDesc,NULL));
		
		return tex;
	}
}textureManager;
