#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iostream>
#include <mesh.h>
#include <nlohmann/json.hpp>


unordered_map<string,LoadedMesh*> loaded_meshs;

void process_assimp_scene(const aiScene*,vector<Vertex>&,vector<int>&);

Mesh::Mesh(Scene* scene, json& desc)
{
    const string& file=desc["file"].get<string>();
    if(loaded_meshs.find(file)==loaded_meshs.end()){
        load_mesh(file);
    }
    mesh=loaded_meshs[file];
}

void Mesh::load_mesh(const string& file)
{
    Assimp::Importer importer;
    auto scene = importer.ReadFile(file.c_str(), 0);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        cout << "error: failed to load mesh" << endl;
        exit(0);
    }

    LoadedMesh* mesh = new LoadedMesh;
    process_assimp_scene(scene,mesh->vertices,mesh->indices);
    build_mesh_data(mesh->vertices,mesh->indices,mesh->mesh_data);

    loaded_meshs.insert({file,mesh});
}

void process_assimp_scene(const aiScene* scene,vector<Vertex>& vertices,vector<int>& indices){
    auto mesh = scene->mMeshes[0];
    int vertex_cnt = mesh->mNumVertices;
    int index_cnt = mesh->mNumFaces * 3;

    auto positions = mesh->mVertices;
    auto normals = mesh->mNormals;
    auto tex_coords = mesh->mTextureCoords[0];
    auto faces = mesh->mFaces;

    if (!positions || !normals || !tex_coords) {
        cout << "invaild mesh" << endl;
        exit(0);
    }

    vertices.resize(vertex_cnt);
    for (int i = 0; i < vertex_cnt; i++) {
        vertices[i] = {
            .p = {.x=positions[i].x, .y=positions[i].y, .z=positions[i].z},
            .n = {.x=normals[i].x, .y=normals[i].y, .z=normals[i].z},
            .uv = {.x=tex_coords[i].x, .y=tex_coords[i].y},
        };
    }
    indices.resize(index_cnt);
    for (int i=0; i<index_cnt; i+=3) {
        if (faces[i/3].mNumIndices!=3) {
            cout << "invaild mesh: not triangle" << endl;
            exit(0);
        }
        indices[i] = faces[i/3].mIndices[0];
        indices[i+1] = faces[i/3].mIndices[1];
        indices[i+2] = faces[i/3].mIndices[2];
    }
}

extern "C" EXPORT SceneNode* create(Scene* scene, json& desc)
{
    return new Mesh(scene, desc);
}