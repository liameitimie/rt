#include <iostream>
#include <fstream>
#include <scene.h>
#include <nlohmann/json.hpp>

#include <shape.h>

using namespace std;

int main(){
    ifstream in("asset/scene.json");
    json desc=json::parse(in);
    Scene* scene=Scene::create(desc);

    return 0;
}