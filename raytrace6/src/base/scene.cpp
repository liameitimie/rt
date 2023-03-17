#include <iostream>
#include <scene.h>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <windows.h>

using namespace std;

namespace scene_module{
    unordered_map<string,void*> loaded_modules;

    void* get_module(SceneNodeTag tag,string impl_type){
        string module_name=string(scene_node_tag_name(tag))+"-"+impl_type;
        if(auto it=loaded_modules.find(module_name);it!=loaded_modules.end()){
            return it->second;
        }
        void* module=LoadLibraryA(module_name.c_str());
        loaded_modules.emplace(module_name,module);
        return module;
    }

    Scene::NodeCreater get_node_creater(SceneNodeTag tag,string impl_type){
        void* module=get_module(tag,impl_type);
        return (Scene::NodeCreater)GetProcAddress((HMODULE)module,"create");
    }
}

SceneNode* Scene::load_node(SceneNodeTag tag,json& desc){
    string tag_name=desc["tag"].get<string>();
    if(tag_name!=scene_node_tag_name(tag)){
        cerr<<"error in load_node: tag_name "<<tag_name<<" is don't match tag "<<scene_node_tag_name(tag)<<endl;
        exit(0);
        return nullptr;
    }
    std::string impl_type=desc["impl"].get<std::string>();
    NodeCreater create=scene_module::get_node_creater(tag,impl_type);

    return (*create)(this,desc);
}


Scene* Scene::create(json& desc){
    Scene* scene=new Scene;
    auto& shape_desc_list=desc["shapes"];

    scene->shapes.resize(shape_desc_list.size());
    for(auto& shape_desc:shape_desc_list){
        Shape* shape=(Shape*)scene->load_node(SHAPE,shape_desc);
        scene->shapes.push_back(shape);
    }
    return scene;
}