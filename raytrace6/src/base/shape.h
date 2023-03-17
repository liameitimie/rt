#pragma once

#include <scene_node.h>
#include <accel.h>

class Surface;

class EXPORT Shape: public SceneNode{
public:
    Surface* surface;

    virtual MeshData get_mesh_data();
    // virtual std::vector<Vertex>& vertices();
    // virtual std::vector<int>& indices();
};