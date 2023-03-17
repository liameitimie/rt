#include <bits/stdc++.h>
#define uint unsigned int
using namespace std;

uint D,S,A,M[100];
char str[5];
uint v[300][32];

int main(){
	//第一维直接 2^i 
	for(int i=31;i>=0;i--) cout<<(1u<<i)<<", ";
	cout<<endl;
//	cout<<endl<<endl;
	
	freopen("new-joe-kuo-6.21201","r",stdin);
	cin>>str>>str>>str>>str;
	
	for(int ti=2;ti<=200;ti++){
		cin>>D>>S>>A;
//		cout<<D<<' '<<S<<' '<<A<<endl;
		for(int i=1;i<=S;i++) cin>>M[i];
		
		uint a[100]={0},A_=A,p=S-1;
		do{
			a[p--]=A_&1;
			A_>>=1;
		}while(A_);
		
		for(int k=S+1;k<=32;k++){
			uint M_k=0;
			for(uint i=1;i<=S-1;i++){
				M_k^=(1<<i)*a[i]*M[k-i];
			}
			M_k^=((1<<S)*M[k-S])^M[k-S];
			M[k]=M_k;
		}
		
		for(uint k=1;k<=32;k++){
			v[D][k-1]=M[k]*(1<<(32-k));
		}
		
		for(int k=0; k<32; k++) {
			cout<<v[D][k]<<", ";
		}
		cout<<endl;
//		cout<<endl<<endl;
	}
	
	return 0;
}
