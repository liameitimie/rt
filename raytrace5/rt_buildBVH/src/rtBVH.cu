#include <cuda_runtime.h>
#include <algorithm>
#include <vector>
#include <queue>
#include "rt_api.h"
#include "helper_math.h"
#include "myMath.h"

using std::vector;
using std::queue;

extern "C" 
void rtGetAccelerationStructurePrebuildInfo(
	rtAccelerationStructureBuildInputs* pInputs,
	rtAccelerationStructurePrebuildInfo* pInfo
){
	
}

/*
node:
	float4 x:c0minx y:c0maxx z:c0miny w:c0maxy
	float4 x:c1minx y:c1maxx z:c1miny w:c1maxy
	float4 x:c0minz y:c0maxz z:c1minz w:c1maxz
	float4 x(int):c0-nodeid|~c0-triid y(int):c1-nodeid|~c1-triid
	
woopifiedtri: (Woop's unit triangle intersection test [Woop 2004])
	float4 v0
	float4 v1
	float4 v2
	... other tri
	float4 x:0x80000000(-0f)  terminator

*/

struct aabb
{
	float3 pmin;
	float3 pmax;
	aabb(){pmin=make_float3(1e18),pmax=make_float3(-1e18);}
	aabb(float3 p){pmin=p,pmax=p;}
	aabb(float3 p1,float3 p2){
		pmin=make_float3(min(p1.x,p2.x),min(p1.y,p2.y),min(p1.z,p2.z));
		pmax=make_float3(max(p1.x,p2.x),max(p1.y,p2.y),max(p1.z,p2.z));
	}
	aabb(float3 p1,float3 p2,float3 p3){
		*this=merge(aabb(p1,p2),p3);
	}
	friend aabb merge(aabb a,aabb b){
		aabb c;
		c.pmin=make_float3(min(a.pmin.x,b.pmin.x),min(a.pmin.y,b.pmin.y),min(a.pmin.z,b.pmin.z));
		c.pmax=make_float3(max(a.pmax.x,b.pmax.x),max(a.pmax.y,b.pmax.y),max(a.pmax.z,b.pmax.z));
		return c;
	}
	float SurfaceArea(){
		float3 d=pmax-pmin;
		return 2*(d.x*d.y+d.x*d.z+d.y*d.z);
	}
	int MaxDim(){
		float3 d=pmax-pmin;
		if(d.x>d.y&&d.x>d.z) return 0;
		else if(d.y>d.z) return 1;
		else return 2;
	}
	float3 Centroid(){
		return (pmin+pmax)*0.5;
	}
};

struct wooptri
{
	aabb bounds;
	int id;
	float4 v0,v1,v2;
};

void woopifyTri(
	float3 v0,float3 v1,float3 v2,
	float4& woopv0,float4& woopv1,float4& woopv2
){
	float4x4 mtx;
	mtx.setCol(0,make_float4(v0-v2,0));
	mtx.setCol(1,make_float4(v1-v2,0));
	mtx.setCol(2,make_float4(cross(v0-v2,v1-v2),0));
	mtx.setCol(3,make_float4(v2,1));
	
	mtx.invert();
	
	woopv0=mtx.getRow(2);
	woopv1=mtx.getRow(0);
	woopv2=mtx.getRow(1);
}

void initTri(rtAccelerationStructureDesc &Desc,vector<wooptri>& tris)
{
	rtGeometryDesc* p=Desc.Inputs.pGeometryDescs;
	int nGeo=Desc.Inputs.NumDescs;
	vector<int> idxbuf;
	vector<float3> vtxbuf;
	
	tris.clear();
	for(int i=0;i<nGeo;i++){
		int idxcnt=p[i].IndexCount;
		int vtxcnt=p[i].VertexCount;
		idxbuf.resize(idxcnt);
		vtxbuf.resize(vtxcnt);
		rtCopyFromBuffer(p[i].IndexBuffer,idxbuf.data(),sizeof(int)*idxcnt);
		rtCopyFromBuffer(p[i].VertexBuffer,vtxbuf.data(),sizeof(float3)*vtxcnt);
		
		for(int j=0;j+2<idxcnt;j+=3){
			//assert(idxbuf[j]<vtxcnt&&idxbuf[j+1]<vtxcnt&&idxbuf[j+2]<vtxcnt);
			float3 v0=vtxbuf[idxbuf[j]];
			float3 v1=vtxbuf[idxbuf[j+1]];
			float3 v2=vtxbuf[idxbuf[j+2]];
			wooptri tri;
			tri.bounds=aabb(v0,v1,v2);
			tri.id=tris.size();
			woopifyTri(v0,v1,v2,tri.v0,tri.v1,tri.v2);
			tris.push_back(tri);
		}
	}
}

void buildBVH(vector<wooptri>& tris,vector<float4>& nodedata,vector<float4>& tridata)
{
	if (tris.size() <= 4) {
		aabb c0, c1;
		for (int i = 0; i < tris.size(); i++) {
			c0 = merge(c0, tris[i].bounds);
		}
		nodedata.push_back(make_float4(c0.pmin.x, c0.pmax.x, c0.pmin.y, c0.pmax.y));
		nodedata.push_back(make_float4(c1.pmin.x, c1.pmax.x, c1.pmin.y, c1.pmax.y));
		nodedata.push_back(make_float4(c0.pmin.z, c0.pmax.z, c1.pmin.z, c1.pmax.z));
		nodedata.push_back(make_float4(0, 0, 0, 0));
		int nodeidx = ~0;
		nodedata[3].x = *(float*)(&nodeidx);
		nodedata[3].y = *(float*)(&nodeidx);
		for (int i = 0; i < tris.size(); i++) {
			tridata.push_back(tris[i].v0);
			tridata.push_back(tris[i].v1);
			tridata.push_back(tris[i].v2);
		}
		uint terminator = 0x80000000;
		tridata.push_back(make_float4(*(float*)(&terminator), 0, 0, 0));
		return;
	}

	struct bucketinfo{
		int cnt;
		aabb bounds;
	};
	struct qnode{
		int l,r,dep;
		int fa,ci;
	};
	queue<qnode> q;
	const int nbuckets=12;
	bucketinfo buckets[nbuckets];
	
	q.push(qnode{0,(int)tris.size(),0,-1,0});
	while(!q.empty()){
		
		qnode u=q.front(); q.pop();
		//assert(u.dep<64);
		
		if(u.r-u.l>4){
			aabb cb;
			for(int i=u.l;i<u.r;i++){
				cb=merge(cb,tris[i].bounds.Centroid());
			}
			int dim=cb.MaxDim();
			
			for(int i=0;i<nbuckets;i++){
				buckets[i].cnt=0,buckets[i].bounds=aabb();
			}
			for(int i=u.l;i<u.r;i++){
				float3 c = tris[i].bounds.Centroid();
				float l=(&cb.pmin.x)[dim];
				float r=(&cb.pmax.x)[dim];
				float x=(&c.x)[dim];
				int b=nbuckets*(x-l)/(r-l);
				if(b==nbuckets) b--;
				buckets[b].cnt++;
				buckets[b].bounds=merge(buckets[b].bounds,tris[i].bounds);
			}
			
			float mincost=1e18;
			int mincostidx=0;
			aabb c0,c1;
			
			for(int i=0;i<nbuckets;i++){
				aabb b0,b1;
				int cnt0=0,cnt1=0;
				for(int j=0;j<=i;j++){
					b0=merge(b0,buckets[j].bounds);
					cnt0+=buckets[j].cnt;
				}
				for(int j=i+1;j<nbuckets;j++){
					b1=merge(b1,buckets[j].bounds);
					cnt1+=buckets[j].cnt;
				}
				float cost=cnt0*b0.SurfaceArea()+cnt1*b1.SurfaceArea();
				if(cost<mincost){
					mincost=cost;
					mincostidx=i;
					c0=b0,c1=b1;
				}
			}
			wooptri *pmid=std::partition(&tris[u.l],&tris[u.r-1]+1,
				[=](wooptri &pi){
					float3 c = pi.bounds.Centroid();
					float l=(&cb.pmin.x)[dim];
					float r=(&cb.pmax.x)[dim];
					float x=(&c.x)[dim];
					int b=nbuckets*(x-l)/(r-l);
					if(b==nbuckets) b--;
					return b<=mincostidx;
				});
			int mid=pmid-&tris[u.l];
			
			int nodeidx=nodedata.size();
			if(u.fa>=0){
				if(u.ci==0) nodedata[u.fa+3].x=*(float*)(&nodeidx);
				else nodedata[u.fa+3].y=*(float*)(&nodeidx);
			}
			nodedata.push_back(make_float4(c0.pmin.x,c0.pmax.x,c0.pmin.y,c0.pmax.y));
			nodedata.push_back(make_float4(c1.pmin.x,c1.pmax.x,c1.pmin.y,c1.pmax.y));
			nodedata.push_back(make_float4(c0.pmin.z,c0.pmax.z,c1.pmin.z,c1.pmax.z));
			nodedata.push_back(make_float4(0,0,0,0));
			
			q.push({u.l,mid,u.dep+1,nodeidx,0});
			q.push({mid,u.r,u.dep+1,nodeidx,1});
		}
		else{
			int nodeidx=tridata.size();
			nodeidx=~nodeidx;
			if(u.fa>=0){
				if(u.ci==0) nodedata[u.fa+3].x=*(float*)(&nodeidx);
				else nodedata[u.fa+3].y=*(float*)(&nodeidx);
			}
			for(int i=u.l;i<u.r;i++){
				tridata.push_back(tris[i].v0);
				tridata.push_back(tris[i].v1);
				tridata.push_back(tris[i].v2);
			}
			uint terminator=0x80000000;
			tridata.push_back(make_float4(*(float*)(&terminator),0,0,0));
		}
		
	}
}

extern "C"
void rtBuildAccelerationStructure(rtAccelerationStructureDesc Desc)
{
	vector<wooptri> tris;
	initTri(Desc,tris);
	
	vector<float4> nodedata;
	vector<float4> tridata;
	
	buildBVH(tris,nodedata,tridata);
	
}
