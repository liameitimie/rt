#pragma once

#include <vector>
#include <span>
#include <def.h>

struct Vertex{
    struct{
        float x,y,z;
    }p,n;
    struct{
        float x,y;
    }uv;
};

struct MeshData{
    std::span<Vertex> vertices;
    std::span<int> indices;
    uint64 vbuffer;
    uint64 ibuffer;
};


class Accel{
public:
};