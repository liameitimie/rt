#include <algorithm>
#include <mesh.h>
#include <cuda_runtime.h>

using namespace std;

void Mesh::build_mesh_data(vector<Vertex>& vertices,vector<int>& indices,MeshData& mesh_data){
    void* vbuffer;
    void* ibuffer;

    uint64 vbuffer_sz=sizeof(Vertex)*vertices.size();
    uint64 ibuffer_sz=sizeof(int)*indices.size();
    cudaMalloc(&vbuffer,vbuffer_sz);
    cudaMalloc(&ibuffer,ibuffer_sz);
    cudaMemcpy(vbuffer,vertices.data(),vbuffer_sz,cudaMemcpyHostToDevice);
    cudaMemcpy(ibuffer,indices.data(),ibuffer_sz,cudaMemcpyHostToDevice);

    mesh_data={
        span<Vertex>(vertices),
        span<int>(indices),
        (uint64)vbuffer,
        (uint64)ibuffer,
    };
}