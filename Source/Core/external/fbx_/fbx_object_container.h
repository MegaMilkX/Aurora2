#ifndef FBX_OBJECT_CONTAINER_H
#define FBX_OBJECT_CONTAINER_H

#include "fbx_object.h"
#include "fbx_type_index.h"
#include <map>
#include <vector>
#include <memory>

namespace Fbx {

class ObjectContainer {
public:
    template<typename T>
    T* Create(Node& n) {
        if(!T::IdentifyNode(n)) {
            return 0;
        }
        std::shared_ptr<Object> ptr(new T());
        ptr->Make(n);
        objects[TypeInfo<T>::Index()].emplace_back(ptr);
        return ptr.get();
    }
private:
    std::map<
        TypeIndex, 
        std::vector<
            std::shared_ptr<Object>
        >
    > objects;
};

}

#endif
