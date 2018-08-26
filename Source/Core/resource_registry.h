#ifndef RESOURCE_REGISTRY_H
#define RESOURCE_REGISTRY_H

#include "resources/resource_raw.h"
#include <map>
#include <string>

class ResourceRegistry
{
public:
    ~ResourceRegistry()
    {
        for(auto& kv : resources)
        {
            delete kv.second;
        }
    }
    void Add(const std::string& name, ResourceRaw* raw)
    {
        //if(Exists(name)) delete resources[name];
        if(!Exists(name))
        {
            resources[name] = raw;
        }
    }
    bool Exists(const std::string& name)
    {
        return resources.count(name) != 0;
    }
    ResourceRaw* Get(const std::string& name)
    {
        if(!Exists(name)) return 0;
        return resources[name];
    }
private:
    std::map<std::string, ResourceRaw*> resources;
};

extern ResourceRegistry g_resourceRegistry;

#endif
