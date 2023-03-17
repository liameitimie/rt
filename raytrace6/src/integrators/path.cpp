#include <integrator.h>

class Path: public Integrator{
public:
    int depth;

    Path(Scene* scene,json& desc){

    }
};

extern "C"
EXPORT SceneNode* create(Scene* scene,json& desc){
    return new Path(scene,desc);
}