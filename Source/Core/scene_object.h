#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

#include <vector>
#include <map>

#include "typeindex.h"

#include "external/json.hpp"

#include <blueprint.h>

#undef GetObject

class SceneObject
{
public:
    class Component
    {
    friend SceneObject;
    public:
        virtual ~Component() {}
        SceneObject* Object() { return object; }
        SceneObject* GetObject() { return object; }
        template<typename T>
        T* GetComponent()
        { return GetObject()->GetComponent<T>(); }
        template<typename T>
        T* Get() { return GetObject()->GetComponent<T>(); }
        template<typename T>
        T* RootGet() { return Object()->Root()->GetComponent<T>(); }
        
        virtual void OnInit() {}
        virtual std::string Serialize() { return "{}"; }
        virtual void Deserialize(const std::string& data) {}
    private:
        SceneObject* object;
    };
    
    SceneObject() : parentObject(0) {}
    SceneObject(SceneObject* parent) : parentObject(parent) {}
    ~SceneObject()
    {
        for(unsigned i = 0; i < objects.size(); ++i)
            delete objects[i];
        for(auto& kv : components)
        {
            delete kv.second;
        }
    }
    
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

    SceneObject* Instantiate(Blueprint& blueprint)
    {
        std::cout << "Instantiating blueprint..." << std::endl;
        SceneObject* o = CreateObject();
        o->Name(blueprint.Name());
        for(auto& kv : blueprint.GetComponents())
        {
            Component* c = o->Get(kv.second.meta.Name());
        }
        return o;
    }
    
    SceneObject* CreateObject()
    {
        SceneObject* o = new SceneObject(this);
        objects.push_back(o);
        return o;
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
            c->object = this;
            components.insert(std::make_pair(TypeInfo<T>::Index(), c));
            c->OnInit();
            return c;
        }
        else
            return c;
    }
    Component* Get(const std::string& component)
    {
        Meta meta = Meta::Get(component);
        typeindex type = meta.TypeIndex();
        Component* c = FindComponent(type);
        if(!c)
        {
            c = (Component*)meta.Create();
            c->object = this;
            components.insert(std::make_pair(type, c));
            c->OnInit();
            return c;
        }
        else
            return c;
    }
    
    template<typename T>
    T* FindComponent()
    {
        return (T*)FindComponent(TypeInfo<T>::Index());
    }

    Component* FindComponent(typeindex t)
    {
        std::map<typeindex, Component*>::iterator it;
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
    
    void Name(const std::string& name) { this->name = name; }
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
    
    // Serialization
    template<typename T>
    static void RegisterComponent(const std::string& name)
    {
        headerStaticWrap<SceneObject>::compAllocFuncs[name] = &GetComponentBase<T>;
        headerStaticWrap<SceneObject>::compTypeIndexToName[TypeInfo<T>::Index()] = name;
    }
    std::string Serialize()
    {
        using json = nlohmann::json;
        json j = json::object();
        
        j["Name"] = name;
        
        for(auto& kv : components)
        {
            std::string comp_data = 
                kv.second->Serialize();
            json j2 = json::parse(comp_data);
            auto& it = headerStaticWrap<SceneObject>::compTypeIndexToName.find(kv.first);
            if(it == headerStaticWrap<SceneObject>::compTypeIndexToName.end())
                continue;
            j[it->second] = j2;
        }
        
        j["Objects"] = json::array();
        for(auto& object : objects)
        {
            std::string obj_data = 
                object->Serialize();
            j["Objects"].push_back(json::parse(obj_data));
        }
        
        return j.dump();
    }
    void Deserialize(const std::string& data)
    {
        
    }
private:
    template<typename T>
    Component* GetComponentBase() { return GetComponent<T>(); }
    typedef Component*(SceneObject::*FuncGetComponent_t)();
    template<typename T>
    struct headerStaticWrap{
        static std::map<std::string, FuncGetComponent_t> compAllocFuncs;
        static std::map<typeindex, std::string> compTypeIndexToName;
    };
    Component* GetComponentByName(const std::string& name)
    {
        std::map<std::string, FuncGetComponent_t>::iterator it =
            headerStaticWrap<SceneObject>::compAllocFuncs.find(name);
        if(it == headerStaticWrap<SceneObject>::compAllocFuncs.end())
        {
            return 0;
        }
        
        FuncGetComponent_t get_component = 
            it->second;
        return (this->*get_component)();
    }
    
    std::string name;
    SceneObject* parentObject;
    std::vector<SceneObject*> objects;
    std::map<typeindex, Component*> components;
};

template<typename T>
std::map<std::string, SceneObject::FuncGetComponent_t> SceneObject::headerStaticWrap<T>::compAllocFuncs;
template<typename T>
std::map<typeindex, std::string> SceneObject::headerStaticWrap<T>::compTypeIndexToName;

#endif
