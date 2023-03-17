#pragma once

#include <scene_node.h>
#include <vector>

class Camera;
class Integrator;
class Shape;

class EXPORT Scene{
public:
    Integrator* integrator;
    Camera* camera;
    std::vector<Shape*> shapes;

    using NodeCreater=SceneNode*(*)(Scene*,json&);
    SceneNode* load_node(SceneNodeTag tag,json& desc);
    static Scene* create(json& desc);
};