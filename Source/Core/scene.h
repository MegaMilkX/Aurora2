#ifndef TESTSCENE_H
#define TESTSCENE_H

#include <rttr/type>
#include <map>
#include <memory>

typedef int32_t h_object;
typedef int32_t h_component;

namespace TEST {

class Component {
public:
    virtual ~Component() {}
};

class Transform : public Component {
public:
};

class Scene {
public:
    struct Object {
        h_object parent;
        std::map<rttr::type, h_component> components;
    };

    h_object CreateObject() {
        h_object h = objects.size();
        objects.emplace_back(Object());
        return h;
    }
    template<typename T>
    T* GetComponent(h_object hobj) {
        h_component hcomp = 0;
        if(!HasComponent<T>(hobj)) {
            hcomp = CreateComponent<T>(hobj);
        } else {
            hcomp = objects[hobj].components[rttr::type::get<T>()];
        }
        return (T*)pools[rttr::type::get<T>()][hcomp].get();
    }
    
    template<typename T>
    h_component HasComponent(h_object hobj) {
        return objects[hobj].components.count(rttr::type::get<T>()) != 0;
    }
    template<typename T>
    h_component CreateComponent(h_object hobj) {
        h_component h = CreateComponent<T>();
        objects[hobj].components[rttr::type::get<T>()] = h;
        return h;
    }
    template<typename T>
    h_component CreateComponent() {
        h_component h = pools[rttr::type::get<T>()].size();
        pools[rttr::type::get<T>()].emplace_back(std::shared_ptr<Component>(new T()));
        return h;
    }
private:
    std::vector<Object> objects;
    std::map<
        rttr::type, 
        std::vector<
            std::shared_ptr<Component>
        >
    >                   pools;
};

}

#endif
