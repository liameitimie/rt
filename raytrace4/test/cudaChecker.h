#pragma once

#define CHECK(call)\
{\
    const cudaError_t error=call;\
    if(error!=cudaSuccess)\
    {\
        printf("ERROR: %s:%d,",__FILE__,__LINE__);\
        printf("code:%d,reason:%s\n",error,cudaGetErrorString(error));\
        exit(1);\
    }\
}
#define CHECK_KERNEL()\
{\
	CHECK(cudaDeviceSynchronize());\
	const cudaError_t error=cudaGetLastError();\
	if(error!=cudaSuccess)\
    {\
        printf("ERROR: %s:%d,",__FILE__,__LINE__);\
        printf("code:%d,reason:%s\n",error,cudaGetErrorString(error));\
        exit(1);\
    }\
}

