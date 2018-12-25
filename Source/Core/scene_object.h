#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

#include <vector>
#include <map>
#include <memory>

#include "typeindex.h"

#include "../general/util.h"
#include <util/split.h>
#include <rttr/registration>
#include <rttr/registration_friend>
#include <rttr/type>
#include <util/static_run.h>

#include "external/json.hpp"

#undef GetObject

class UnserializableComponentToken {};

#define CLONEABLE \
public: \
    Component* clone() { \
        return new std::remove_reference<decltype(*this)>::type(*this); \
    }

class SceneController;
class Component;
class SceneObject : public std::enable_shared_from_this<SceneObject>
{
    friend Component;
public:    
    static std::shared_ptr<SceneObject> Create() {
        return std::make_shared<SceneObject>();
    }

    // Don't use
    SceneObject() 
    : SceneObject(0) {}

    SceneObject(SceneObject* parent) 
    : parentObject(parent),
    name("New Object"),
    controller(0) {
        static int64_t next_uid = 0;
        uid = ++next_uid;
    }
    // ---

public:
    ~SceneObject();
    
    SceneObject*            Root();
    SceneObject*            Parent();
    bool                    IsRoot();
    SceneObject*            CreateObject();
    void                    Erase(SceneObject* child);
    void                    Destroy();
    template<typename T>
    T*                      RootGet();
    template<typename T>
    T*                      Get();
    template<typename T>
    T*                      GetComponent();
    Component*              Get(const std::string& component);
    template<typename T>
    T*                      FindComponent();
    Component*              FindComponent(rttr::type t);
    template<typename T>
    std::vector<T*>         FindAllOf();
    int64_t                 Uid() const;
    void                    Name(const std::string& name);
    std::string             Name() const;
    SceneObject*            FindObject(const std::string& name);
    SceneObject*            FindChild(const std::string& name);
    SceneObject*            CreateFrom(SceneObject* from);
    
    unsigned int            ChildCount() const;
    SceneObject*            GetChild(unsigned int i) const;
    unsigned int            ComponentCount() const;
    Component*              GetComponent(unsigned int id) const ;

    void                    SetController(SceneController* con);
    SceneController*        GetController();

    std::weak_ptr<SceneObject> WeakPtr();
private:
    void AddComponent(Component* c, rttr::type t);
    void RefreshComponent(Component* com);

    template<typename T>
    Component* GetComponentBase() { return GetComponent<T>(); }
    typedef Component*(SceneObject::*FuncGetComponent_t)();
    template<typename T>
    struct headerStaticWrap{
        static std::map<std::string, FuncGetComponent_t> compAllocFuncs;
        static std::map<typeindex, std::string> compTypeIndexToName;
    };
    
    int64_t uid;
    std::string name;
    SceneObject* parentObject;
    std::vector<std::shared_ptr<SceneObject>> objects;
    std::map<rttr::type, Component*> components;
    SceneController* controller;
};

template<typename T>
T* SceneObject::RootGet() { 
    return Root()->GetComponent<T>(); 
}
template<typename T>
T* SceneObject::Get() { 
    return GetComponent<T>(); 
}
template<typename T>
T* SceneObject::GetComponent() {
    T* c = FindComponent<T>();
    if (!c)
    {
        c = new T();
        c->type = rttr::type::get<T>();
        AddComponent(c, rttr::type::get<T>());
        return c;
    }
    else
        return c;
}
template<typename T>
T* SceneObject::FindComponent() {
    return (T*)FindComponent(rttr::type::get<T>());
}

template<typename T>
std::vector<T*> SceneObject::FindAllOf() {
    std::vector<T*> result;
    T* c = FindComponent<T>();
    if(c) result.push_back(c);
    for(unsigned i = 0; i < objects.size(); ++i)
    {
        std::vector<T*> r = objects[i]->FindAllOf<T>();
        result.insert(result.end(), r.begin(), r.end());
    }
    return result;
}

template<typename T>
std::map<std::string, SceneObject::FuncGetComponent_t> SceneObject::headerStaticWrap<T>::compAllocFuncs;
template<typename T>
std::map<typeindex, std::string> SceneObject::headerStaticWrap<T>::compTypeIndexToName;

#endif
