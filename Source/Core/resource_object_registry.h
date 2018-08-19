#ifndef RESOURCE_OBJECT_REGISTRY_H
#define RESOURCE_OBJECT_REGISTRY_H

#include <string>
#include <map>
#include <memory>
#include <rttr/type>

#include "resources/resource_object.h"

class ResourceObjectRegistry
{
public:
    template<typename T>
    std::shared_ptr<ResourceObject> Create(const std::string& name)
    {
        ResourceRaw* raw = g_resourceRegistry.Get(name);
        if(!raw)
        {
            return 0;
        }
        ResourceObject* ro = new T();
        ro->Build(raw);
        auto key_pair = std::make_pair(rttr::type::get<T>(), name);
        resources[key_pair].reset(ro);
        return resources[key_pair];
    }
    template<typename T>
    bool Exists(const std::string& name)
    {
        auto key_pair = std::make_pair(rttr::type::get<T>(), name);
        if(resources.count(key_pair) != 0)
            return true;
        else
            return false;
    }
    template<typename T>
    T* Get(const std::string& name)
    {
        if(!Exists<T>(name)) {
            auto result = Create<T>(name);
            if(!result.get()) return 0;
            return (T*)result.get();
        }
        auto key_pair = std::make_pair(rttr::type::get<T>(), name);
        return (T*)resources[key_pair].get();
    }
private:
    std::map<
        std::pair<rttr::type, std::string>, 
        std::shared_ptr<ResourceObject>
    > resources;
};

extern ResourceObjectRegistry g_resourceObjectRegistry;

#endif
