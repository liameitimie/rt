#pragma once

class Scene;

class Pipeline{
public:
    static Pipeline* create(Scene* scene);
};