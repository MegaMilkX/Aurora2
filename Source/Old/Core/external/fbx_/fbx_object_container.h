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
    template<typename T>
    size_t Count() {
        return objects[TypeInfo<T>::Index()].size();
    }
    template<typename T>
    T* Get(size_t i) {
        return (T*)objects[TypeInfo<T>::Index()][i];
    }
    template<typename T>
    T* GetByUid(int64_t uid) {
        for(size_t i = 0; i < objects[TypeInfo<T>::Index()].size(); ++i) {
            Object* o = objects[TypeInfo<T>::Index()][i];
            if(o->GetUid() == uid)
                return (T*)o;
        }
        return 0;
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
