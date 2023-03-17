#include <iostream>
#include "rt_types.h"
#include "rt_api.h"

using namespace std;

int n=5;
int a[]={3,43,1,5,6};
int b[]={4,5,11,23,1};

extern "C" void add(rtBuffer buf1,rtBuffer buf2,rtBuffer buf3,int n);

int main(){
	int size=n*sizeof(int);
	rtBuffer buf1=rtCreateBuffer(size);
	rtBuffer buf2=rtCreateBuffer(size);
	rtBuffer buf3=rtCreateBuffer(size);
	
	rtCopyToBuffer(buf1,a,size);
	rtCopyToBuffer(buf2,b,size);
	
	add(buf1,buf2,buf3,n);
	
	rtCopyFromBuffer(buf3,a,size);
	
	for(int i=0;i<n;i++){
		cout<<a[i]<<endl;
	}
	
	return 0;
}
