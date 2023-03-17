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
		if((imtype>>3)+1==3){//3ͨ����cuda����֧��3ͨ��
			cv::cvtColor(image,image,cv::COLOR_BGR2RGBA);
			imtype=image.type();
		}
		int c=(imtype>>3)+1;//��ɫͨ���� (channel) 
		int t=(imtype&0x07);//opencv��ɫ��ʽ 0:CV_8U, 1:CV8S, 2:CV_16U, 3:CV_16S, 4:CV_32S, 5:CV_32F, 6:CV_64F
		int b=(8<<(t>>1));  //ÿ����ɫͨ����λ�� 
		
		int sz=w*h*c*(b>>3); 
		uchar* p_data=image.data;
		
		/* �������ظ�ʽ���� */
		cudaChannelFormatDesc formatDesc=cudaCreateChannelDesc(
			(c>=1)?b:0, (c>=2)?b:0, (c>=3)?b:0, (c>=4)?b:0,
			((t>4)? cudaChannelFormatKindFloat
				:(((t&1)||t==4)? cudaChannelFormatKindSigned
					:cudaChannelFormatKindUnsigned
				)
			)
		);//����opencv��ɫ��ʽ��������cuda������������ 
		
		/* ����cuda���鲢����ͼ��cuda���� */
		cudaArray_t array;
		CHECK(cudaMallocArray(&array,&formatDesc,w,h));
		CHECK(cudaMemcpyToArray(array,0,0,p_data,sz,cudaMemcpyHostToDevice));
		
		/* ������Դ���� */
		cudaResourceDesc resDesc;                //��Դ������������Դ����������Դ��ַ 
	    memset(&resDesc,0,sizeof(resDesc));
	    resDesc.resType=cudaResourceTypeArray;   //��Դ����Ϊcuda���飬 
	    resDesc.res.array.array=array;           //����֮�⻹��mipmap��linear��pitch2D 
	    
	    /* ������������ */
		cudaTextureDesc texDesc;                 //����������������ֵ�������������� 
		memset(&texDesc,0,sizeof(texDesc));
		texDesc.normalizedCoords=true;           //���ʵ�uv�����Թ�һ��ģʽ���� 
		texDesc.filterMode=cudaFilterModeLinear; //��ֵ����Ϊ���Բ�ֵ��0:�����, 1:���� 
		texDesc.readMode=cudaReadModeNormalizedFloat;//��ȡ�����ݹ�һ�� 
		//td.addressMode[0..2] //��uv����һʱ��ȡ�Ĳ��ԣ�0:wrap, 1:clamp, 2:mirror, 3:border 
		
		/* ����������� */
		cudaTextureObject_t tex=0;
		CHECK(cudaCreateTextureObject(&tex,&resDesc,&texDesc,NULL));
		
		return tex;
	}
}textureManager;
