#ifndef RESOURCE_REF_H
#define RESOURCE_REF_H

#include <resource_object_registry.h>

class ResourceRef
{
public:
    ResourceRef() {}
    ResourceRef(const std::string& name) 
    : name(name) {}
    operator bool() const {
        return g_resourceRegistry.Exists(name);
    }
    void Set(const std::string& name) { this->name = name; }
    const std::string& GetTargetName() const { return name; }
    template<typename T>
    T* Get() {
        return g_resourceObjectRegistry.Get<T>(name);
    }
    ResourceRaw* GetRaw() { return g_resourceRegistry.Get(name); }
private:
    std::string name;
};

#endif
