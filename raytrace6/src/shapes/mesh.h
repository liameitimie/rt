#include <shape.h>
#include <vector>
#include <unordered_map>

using namespace std;

struct LoadedMesh{
    vector<Vertex> vertices;
    vector<int> indices;
    MeshData mesh_data;
};

class Mesh: public Shape{
    LoadedMesh* mesh;
    void load_mesh(const string& file);
    void build_mesh_data(vector<Vertex>& vertices,vector<int>& indices,MeshData& mesh_data);
public:
    Mesh(Scene* scene,json& desc);
    virtual MeshData get_mesh_data() override{
        return mesh->mesh_data;
    }
};

