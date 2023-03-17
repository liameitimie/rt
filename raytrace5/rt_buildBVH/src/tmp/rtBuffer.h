#include <cuda_runtime.h>
#include "cudaChecker.h"
#include "types.h"

struct rtBuffer
{
	uint64 StartAddress;
	uint32 Size;
	uint32 Stride;
};

rtBuffer rtCreateBuffer(uint Size, uint Stride = 0)
{
	void* ptr;
	check(cudaMalloc(&ptr, Size));
	rtBuffer res;
	res.StartAddress = (uint64)ptr;
	res.Size = Size;
	res.Stride = Stride;
	return res;
}

void rtReleaseBuffer(rtBuffer Buffer)
{
	check(cudaFree((void*)Buffer.StartAddress));
}

void rtCopyToBuffer(rtBuffer Buffer, void* ptr, uint Size)
{
	check(cudaMemcpy((void*)Buffer.StartAddress, ptr, Size, cudaMemcpyHostToDevice));
}

void rtCopyFromBuffer(rtBuffer Buffer, void* ptr, uint Size)
{
	check(cudaMemcpy(ptr, (void*)Buffer.StartAddress, Size, cudaMemcpyDeviceToHost));
}

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
