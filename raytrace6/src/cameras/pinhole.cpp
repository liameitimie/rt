#include <camera.h>

struct float3{
    float x,y,z;
};

class Pinhole: public Camera{
public:
    float3 position;
    float3 front;
    float3 right;
    float3 up;
    float fov;

    Pinhole(Scene* scene,json& desc){

    }
};

extern "C"
EXPORT SceneNode* create(Scene* scene,json& desc){
    return new Pinhole(scene,desc);
}