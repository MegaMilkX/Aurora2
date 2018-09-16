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
    virtual std::shared_ptr<Resource> Build(DataSourceRef ref) = 0;
};

template<typename T>
class ResourceBuilder : public IResourceBuilder {
    virtual std::shared_ptr<Resource> Build(const std::string& name) {
        return std::dynamic_pointer_cast<Resource>(GlobalResourceFactory().Get<T>(name));
    }
    virtual std::shared_ptr<Resource> Build(DataSourceRef ref) {
        std::shared_ptr<T> res(new T());
        if(!res->Build(ref)) {
            std::cout << "Failed to build resource of type " << rttr::type::get<T>().get_name().to_string() << std::endl;
        }
        res->Name(ref->Name());
        res->Storage(Resource::LOCAL);
        return std::dynamic_pointer_cast<Resource>(res);
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
    ResourceRef& SetUnsafe(const std::shared_ptr<Resource>& ptr) {
        resource = ptr;
        if(!ptr) {
            std::cout << "ResourceRef::SetUnsafe - resource is null" << std::endl;
        }
        Changed();
        return *this;
    }
    std::shared_ptr<Resource> MakeResource(DataSourceRef data) {
        if(!type.is_valid()) {
            LOG("ResourceRef::MakeResource - type is invalid");
            return std::shared_ptr<Resource>();
        }
        LOG("ResourceRef::MakeResource - type: " << type.get_name().to_string());
        if(!builder) {
            LOG("ResourceRef::MakeResource - no type, can't build")
            return std::shared_ptr<Resource>();
        }
        return builder->Build(data);
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
    std::shared_ptr<Resource> Get() { return resource; }

    void AddChangeCallback(resource_ref_changed_cb_t cb) {
        callbacks.emplace_back(cb);
    }

    bool IsValid() const {
        return type.is_valid();
    }
    void Print() {
        LOG("ResourceRef type: " << type.get_name().to_string());
        LOG("ResourceRef builder available: " << (builder.get()));
        LOG("ResourceRef resource: " << (resource.get()));
    }
private:
    ResourceRef(rttr::type type, std::shared_ptr<IResourceBuilder> builder)
    : type(type), builder(builder) {}

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
