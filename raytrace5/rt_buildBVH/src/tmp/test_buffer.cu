#include <iostream>
#include "rtBuffer.h"

using namespace std;

int n=5;
int a[]={3,43,1,5,6};
int b[]={4,5,11,23,1};

__global__
void kernel(rtBufferView<int> a,rtBufferView<int> b,rtBufferView<int> c,int n){
	int idx=threadIdx.x;
	if(idx<n) c[idx]=a[idx]+b[idx];
}

int main(){
	int size=n*sizeof(int);
	rtBuffer buf1=rtCreateBuffer(size);
	rtBuffer buf2=rtCreateBuffer(size);
	rtBuffer buf3=rtCreateBuffer(size);
	
	rtCopyToBuffer(buf1,a,size);
	rtCopyToBuffer(buf2,b,size);
	
	kernel<<<1,n>>>(buf1,buf2,buf3,n);
	check_kernel();
	
	rtCopyFromBuffer(buf3,a,size);
	
	for(int i=0;i<n;i++){
		cout<<a[i]<<endl;
	}
	
	return 0;
}
