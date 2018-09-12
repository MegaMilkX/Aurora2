#ifndef RESOURCE_FACTORY_H
#define RESOURCE_FACTORY_H

#include "resource.h"
#include "data_registry.h"
#include <unordered_map>

class ResourceFactory {
public:
    typedef std::weak_ptr<Resource> res_weak_ptr_t;
    typedef std::shared_ptr<Resource> res_shared_ptr_t;
    typedef std::unordered_map<std::string, res_weak_ptr_t> resource_map_t;
    
    template<typename T>
    std::shared_ptr<T> Get(const std::string& name) {
        DataSourceRef dataSrc = GlobalDataRegistry().Get(name);
        if(!dataSrc) {
            std::cout << "Data source " << name << " doesn't exist." << std::endl;
            return std::shared_ptr<T>();
        }
        std::shared_ptr<T> ptr;
        res_weak_ptr_t& weak = resources[name];
        if(weak.expired()) {
            ptr.reset(new T());
            if(!ptr->Build(dataSrc)) {
                std::cout << "Failed to build resource " << name << std::endl;
                return std::shared_ptr<T>();
            }
            weak = ptr;
            ptr->Name(name);
            ptr->Storage(Resource::GLOBAL);
        } else {
            ptr = std::dynamic_pointer_cast<T>(weak.lock());
        }
        return ptr;
    }
private:
    resource_map_t resources;
};

inline ResourceFactory& GlobalResourceFactory() {
    static ResourceFactory f;
    return f;
}

#endif
