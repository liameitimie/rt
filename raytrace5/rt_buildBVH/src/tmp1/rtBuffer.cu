#include <cuda_runtime.h>
#include "rt_types.h"
#include "cudaChecker.h"

extern "C"
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

extern "C"
void rtReleaseBuffer(rtBuffer Buffer)
{
	check(cudaFree((void*)Buffer.StartAddress));
}

extern "C"
void rtCopyToBuffer(rtBuffer Buffer, void* ptr, uint Size)
{
	check(cudaMemcpy((void*)Buffer.StartAddress, ptr, Size, cudaMemcpyHostToDevice));
}

extern "C"
void rtCopyFromBuffer(rtBuffer Buffer, void* ptr, uint Size)
{
	check(cudaMemcpy(ptr, (void*)Buffer.StartAddress, Size, cudaMemcpyDeviceToHost));
}
