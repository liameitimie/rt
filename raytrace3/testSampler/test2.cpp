#include <iostream>
#include <algorithm>
#include "../Sampler.h"

using namespace std;

float randf(){
	return (float)rand()/RAND_MAX;
}

float3 v=normalize(make_float3(-2,1,0)),n{0,1,0};
float3 bc{1,1,1};
float m=1,r=0.1;

int main(){
	for(int i=1;i<=10;i++){
		float x1=randf();
		float x2=randf();
		float x3=randf();
		
		float3 l=SpBRDF(x1,x2,x3,v,n,m,r);
		float3 fr=BRDF(v,l,n,bc,m,r);
		float pdf=PdfBRDF(l,v,n,m,r);
		
		cout<<l.x<<' '<<l.y<<' '<<l.z<<endl;
		cout<<fr.x<<' '<<fr.y<<' '<<fr.z<<endl;
		cout<<x1<<' '<<x2<<' '<<x3<<' '<<pdf<<endl<<endl;
		//cout<<
	}
	return 0;
}
