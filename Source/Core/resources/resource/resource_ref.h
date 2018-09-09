#ifndef RESOURCE_REFERENCE_H
#define RESOURCE_REFERENCE_H

#include "resource.h"
#include "resource_factory.h"
#include <rttr/type>
#include <functional>

class IResourceBuilder {
public:
    virtual ~IResourceBuilder() {}
    virtual std::shared_ptr<Resource> Build(const std::string& name) = 0;
};

template<typename T>
class ResourceBuilder : public IResourceBuilder {
    virtual std::shared_ptr<Resource> Build(const std::string& name) {
        return std::dynamic_pointer_cast<Resource>(GlobalResourceFactory().Get<T>(name));
    }
};

class ResourceRef {
public:
    typedef std::function<void(ResourceRef* ref)> resource_ref_changed_cb_t;

    template<typename T>
    static ResourceRef Make() {
        return ResourceRef(
            rttr::type::get<T>(), 
            std::shared_ptr<IResourceBuilder>(new ResourceBuilder<T>())
        );
    }

    operator bool() const {
        return resource.get() != 0;
    }

    bool Set(const std::string& name) {
        if(type == rttr::type::get<void>()) return 0;
        resource = builder->Build(name);
        Changed();
        return true;
    }
    template<typename T>
    ResourceRef& operator=(const std::shared_ptr<T>& ptr) {
        return Set(ptr);
    }
    template<typename T>
    ResourceRef& Set(const std::shared_ptr<T>& ptr) {
        if(type != rttr::type::get<T>()) {
            std::cout << "ResourceRef::Set type mismatch: " <<
                type.get_name().to_string() <<
                " expected, " << 
                rttr::type::get<T>().get_name().to_string() <<
                " provided" << std::endl; 
            return *this;
        }
        std::cout << "ResourceRef::Set " << type.get_name().to_string() << std::endl;
        resource = ptr;
        Changed();
        return *this;
    }
    template<typename T>
    T* Get() {
        if(type != rttr::type::get<T>()) {
            std::cout << "ResourceRef::Get type mismatch: " <<
                type.get_name().to_string() <<
                " expected, " << 
                rttr::type::get<T>().get_name().to_string() <<
                " provided" << std::endl; 
            return 0;
        }
        return (T*)resource.get();
    }

    void AddChangeCallback(resource_ref_changed_cb_t cb) {
        callbacks.emplace_back(cb);
    }
private:
    ResourceRef(rttr::type type, std::shared_ptr<IResourceBuilder> builder)
    : type(type) {}

    void Changed() {
        for(auto& cb : callbacks) {
            cb(this);
        }
    }

    rttr::type type;
    std::shared_ptr<Resource> resource;
    std::shared_ptr<IResourceBuilder> builder;

    std::vector<resource_ref_changed_cb_t> callbacks;
};

#endif
