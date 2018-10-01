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
        T* ptr = n.Convert<T>();
        if(!ptr) return 0;
        objects[TypeInfo<T>::Index()].emplace_back(ptr);
        return ptr;
    }
private:
    std::map<
        TypeIndex, 
        std::vector<
            Object*
        >
    > objects;
};

}

#endif
