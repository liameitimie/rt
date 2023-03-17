#pragma once

enum SceneNodeTag{
    CAMERA,
    SHAPE,
    LIGHT,
    ENVIRONMENT,
    TEXTURE,
};

constexpr const char* scene_node_tag_name(SceneNodeTag tag){
    switch(tag){
        case CAMERA: return "camera";
        case SHAPE: return "shape";
        case LIGHT: return "light";
        case ENVIRONMENT: return "environment";
        case TEXTURE: return "texture";
        default: break;
    }
    return "__invalid__";
}