#ifndef RESOURCE_REF_H
#define RESOURCE_REF_H

#include <resource_object_registry.h>

class ResourceRef
{
public:
    typedef std::function<void(ResourceRef* ref)> resource_ref_changed_cb_t;

    ResourceRef() {}
    ResourceRef(const std::string& name) 
    : name(name) {}
    operator bool() const {
        return g_resourceRegistry.Exists(name);
    }
    void operator=(const ResourceRef& other) {
        Set(other.name);
    }
    void Set(const std::string& name) {
        this->name = name;
        Changed(); 
    }
    const std::string& GetTargetName() const { return name; }
    template<typename T>
    T* Get() {
        return g_resourceObjectRegistry.Get<T>(name);
    }
    ResourceRaw* GetRaw() { return g_resourceRegistry.Get(name); }

    void AddChangeCallback(resource_ref_changed_cb_t cb) {
        callbacks.emplace_back(cb);
    }
private:
    void Changed() {
        for(auto& cb : callbacks) {
            std::cout << "Calling a callback" << std::endl;
            cb(this);
        }
    }
    std::string name;
    std::vector<resource_ref_changed_cb_t> callbacks;
};

#endif
