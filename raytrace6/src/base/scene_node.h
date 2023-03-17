#pragma once
#include <def.h>
#include <string>
#include <nlohmann/json_fwd.hpp>
#include <scene_node_tag.h>

using nlohmann::json;

class Scene;

class EXPORT SceneNode{
public:
    SceneNodeTag tag;
    std::string impl;

};