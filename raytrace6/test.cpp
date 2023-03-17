#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
//#include <co/all.h>

using json=nlohmann::json;

template<class T1,class T2>
bool is_same(){return false;}

template<class T>
bool is_same(){return true;}

int main(){
/*
    fs::file f("asset/test.json",'r');
    
    if(!f) std::cout<<"no"<<std::endl;
    
    fastring s=f.read(f.size());


    json::Json j=json::parse(s);
    
    for (auto it = j.begin(); it != j.end(); ++it) {
        LOG << it.key() << ": " << it.value();
    }
*/
/*
    std::ifstream in("asset/test.json");
    json j=json::parse(in);

    auto t1=j.begin();
    auto t2=j.at("pi");
    auto t3=j.at("answer");
    auto t4=j.items();

    for(auto& [key,value]:j.items()){
        std::cout<<key<<' '<<value<<std::endl;
    }
*/
    return 0;
}