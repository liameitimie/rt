#include <cuda_runtime.h>
#include "rt_types.h"
#include "cudaChecker.h"

template<class T>
struct rtBufferView
{
	uint64 StartAddress;
	uint32 Size;
	uint32 Stride;
	
	rtBufferView(rtBuffer Buffer)
	{
		StartAddress = Buffer.StartAddress;
		Size = Buffer.Size;
		Stride = Buffer.Stride;
	}
	
	__device__ T& operator[](int i)
	{
		return *(T*)(StartAddress+i*(Stride?Stride:sizeof(T)));
	}
};

__global__
void kernel(rtBufferView<int> a,rtBufferView<int> b,rtBufferView<int> c,int n){
	int idx=threadIdx.x;
	if(idx<n) c[idx]=a[idx]+b[idx];
}

extern "C"
void add(rtBuffer buf1,rtBuffer buf2,rtBuffer buf3,int n)
{
	kernel<<<1,n>>>(buf1,buf2,buf3,n);
	check_kernel();
}

