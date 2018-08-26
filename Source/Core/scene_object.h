#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

#include <vector>
#include <map>

#include "typeindex.h"

#include "external/json.hpp"

#include "../general/util.h"

#include <rttr/registration>
#include <rttr/registration_friend>
#include <rttr/type>
#include <util/static_run.h>

#undef GetObject

class UnserializableComponentToken {};

#define CLONEABLE(TYPE) \
    SceneObject::Component* clone() { \
        return new TYPE(*this); \
    }

class SceneController;

class SceneObject
{
public:
    class Component
    {
        RTTR_ENABLE()
    friend SceneObject;
    public:
        Component()
        : type(rttr::type::get(UnserializableComponentToken()))
        {}
        virtual ~Component() {}

        virtual Component* clone() { return 0; }

        SceneObject* Object() { return object; }
        SceneObject* GetObject() { return object; }
        template<typename T>
        T* GetComponent()
        { return GetObject()->GetComponent<T>(); }
        template<typename T>
        T* Get() { return GetObject()->GetComponent<T>(); }
        template<typename T>
        T* RootGet() { return Object()->Root()->GetComponent<T>(); }

        SceneObject* CreateObject() { return GetObject()->CreateObject(); }
        
        rttr::type GetType() const { return type; }

        virtual void OnInit() {}
    protected:
        rttr::type type;
    private:
        SceneObject* object;
    };
    
    SceneObject() 
    : parentObject(0),
    name(MKSTR(this)),
    controller(0) {}

    SceneObject(SceneObject* parent) 
    : parentObject(parent),
    name(MKSTR(this)),
    controller(0) {}

    ~SceneObject();
    
    SceneObject* Root()
    {
        if(!parentObject)
            return this;
        else
            return parentObject->Root();
    }
    
    SceneObject* Parent()
    {
        return parentObject;
    }

    bool IsRoot()
    {
        return this == Root();
    }
    
    SceneObject* CreateObject()
    {
        SceneObject* o = new SceneObject(this);
        o->SetController(controller);
        objects.push_back(o);
        return o;
    }
    void Erase(SceneObject* child)
    {
        for(unsigned i = 0; i < objects.size(); ++i)
        {
            if(objects[i] == child)
            {
                delete child;
                objects.erase(objects.begin() + i);
                break;
            }
        }
    }
    template<typename T>
    T* RootGet() { return Root()->GetComponent<T>(); }
    template<typename T>
    T* Get() { return GetComponent<T>(); }
    template<typename T>
    T* GetComponent()
    {
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
    Component* Get(const std::string& component)
    {
        rttr::type type = rttr::type::get_by_name(component);
        if(!type.is_valid())
        {
            std::cout << component << " is not a valid type" << std::endl;
            return 0;
        }
        Component* c = FindComponent(type);
        if(!c)
        {
            rttr::variant v = type.create();
            if(!v.get_type().is_pointer())
            {
                std::cout << component << " - invalid component type" << std::endl;
                return 0;
            }
            c = v.get_value<Component*>();
            if(!c) return 0;
            c->type = type;
            AddComponent(c, type);
            return c;
        }
        else
            return c;
    }
    
    template<typename T>
    T* FindComponent()
    {
        return (T*)FindComponent(rttr::type::get<T>());
    }

    Component* FindComponent(rttr::type t)
    {
        std::map<rttr::type, Component*>::iterator it;
        it = components.find(t);
        if(it == components.end())
            return 0;
        else
            return it->second;
    }
    
    template<typename T>
    std::vector<T*> FindAllOf()
    {
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
    
    void Name(const std::string& name) 
    { 
        this->name = name;
        if(this->name.empty()) this->name = MKSTR(this);
    }
    std::string Name() const { return name; }
    
    SceneObject* FindObject(const std::string& name)
    {
        SceneObject* o = 0;
        for(unsigned i = 0; i < objects.size(); ++i)
        {
            if(objects[i]->Name() == name)
            {
                o = objects[i];
                break;
            }
            else if(o = objects[i]->FindObject(name))
            {
                break;
            }
        }
        return o;
    }

    SceneObject* FindChild(const std::string& name)
    {
        for(auto so : objects)
        {
            if(so->Name() == name)
            {
                return so;
            }
        }
        return 0;
    }

    SceneObject* CreateFrom(SceneObject* from)
    {
        if(!from) return 0;
        SceneObject* new_object = CreateObject();
        new_object->SetController(controller);
        new_object->Name(from->Name());
        for(auto so : from->objects)
        {
            new_object->CreateFrom(so);
        }
        for(auto kv : from->components)
        {
            Component* c = kv.second->clone();
            if(!c) continue;
            components.insert(
                std::make_pair(
                    kv.first, 
                    c
                )
            );
        }
    }

    unsigned int ChildCount() const { return objects.size(); }
    SceneObject* GetChild(unsigned int i) const { return objects[i]; }
    unsigned int ComponentCount() const { return components.size(); }
    Component* GetComponent(unsigned int id) const 
    {
        Component* c = 0;
        auto it = components.begin();
        for(unsigned i = 0; i < id; ++i)
        {
            it++;
        }
        return it->second;
    }

    void SetController(SceneController* con)
    {
        controller = con;
        for(auto& o : objects)
        {
            o->SetController(con);
        }
    }
private:
    void AddComponent(Component* c, rttr::type t);

    template<typename T>
    Component* GetComponentBase() { return GetComponent<T>(); }
    typedef Component*(SceneObject::*FuncGetComponent_t)();
    template<typename T>
    struct headerStaticWrap{
        static std::map<std::string, FuncGetComponent_t> compAllocFuncs;
        static std::map<typeindex, std::string> compTypeIndexToName;
    };
    
    std::string name;
    SceneObject* parentObject;
    std::vector<SceneObject*> objects;
    std::map<rttr::type, Component*> components;
    SceneController* controller;
};

template<typename T>
std::map<std::string, SceneObject::FuncGetComponent_t> SceneObject::headerStaticWrap<T>::compAllocFuncs;
template<typename T>
std::map<typeindex, std::string> SceneObject::headerStaticWrap<T>::compTypeIndexToName;

#endif
