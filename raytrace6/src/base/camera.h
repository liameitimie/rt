#pragma once

#include <scene_node.h>

struct int2{
    int x,y;
};

class Camera: public SceneNode{
public:
    int2 resolution;
    std::string output_file;
};