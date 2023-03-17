#include <iostream>
#include <cuda_runtime.h>

using namespace std;

using uint64=unsigned long long;
using func_ptr=int(*)(int,int); 

#define check(call)\
{\
    const cudaError_t error=call;\
    if(error!=cudaSuccess)\
    {\
        printf("ERROR: %s:%d,",__FILE__,__LINE__);\
        printf("code:%d,reason:%s\n",error,cudaGetErrorString(error));\
        exit(1);\
    }\
}
#define check_kernel()\
{\
	check(cudaDeviceSynchronize());\
	const cudaError_t error=cudaGetLastError();\
	if(error!=cudaSuccess)\
    {\
        printf("ERROR: %s:%d,",__FILE__,__LINE__);\
        printf("code:%d,reason:%s\n",error,cudaGetErrorString(error));\
        exit(1);\
    }\
}

__global__
void kernel(uint64 p,int a,int b,int* c){
	func_ptr f=(func_ptr)p;
	*c=f(a,b);
}

__device__
int f(int a,int b){
	return a+b;
}

__device__
uint64 f_ptr=(uint64)f;

int main(){
	uint64 f_h;
	check(cudaMemcpyFromSymbol(&f_h,f_ptr,sizeof(f_h)));
	
	int* c_d;
	check(cudaMalloc(&c_d,sizeof(int)));
	
	kernel<<<1,1>>>(f_h,2,3,c_d);
	check_kernel();
	
	int c;
	check(cudaMemcpy(&c,c_d,sizeof(c),cudaMemcpyDeviceToHost));
	
	cout<<c<<endl;
	
	return 0;
}
