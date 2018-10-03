#ifndef FBX_SCENE_2_H
#define FBX_SCENE_2_H

#include <stdint.h>
#include <string>
#include <iostream>
#include <fstream>
#include <functional>

#include "fbx_node.h"
#include "fbx_object_container.h"

#include "fbx_log.h"

#include "fbx_connection.h"
#include "fbx_properties.h"
#include "fbx_model.h"
#include "fbx_pose.h"
#include "fbx_deformer_skin.h"
#include "fbx_deformer_cluster.h"
#include "fbx_deformer_blend_shape.h"
#include "fbx_animation_stack.h"
#include "fbx_animation_layer.h"
#include "fbx_animation_curve_node.h"
#include "fbx_animation_curve.h"
#include "fbx_geometry.h"
#include "fbx_texture.h"
#include "fbx_material.h"
#include "fbx_node_attribute.h"

#include "fbx_connections.h"

namespace Fbx {

typedef uint8_t Byte;
typedef uint32_t Word;
typedef uint64_t DoubleWord;

class Scene {
public:
    bool ReadMem(const char* data, size_t size);
    bool ReadFile(const std::string& filename);

    size_t RootModelCount() const { return rootModels.size(); }
    Model* GetRootModel(size_t i) { return rootModels[i]; }

    ObjectContainer& Objects() { return objects; }
    ConnectionContainer& Connections() { return connections; }
    template<typename T>
    size_t CountChildren(CONNECTION_TYPE type, int64_t parent) {
        size_t sz = connections.CountChildren(type, parent);
        size_t counter = 0;
        for(size_t i = 0; i < sz; ++i) {
            Connection* c = 
                connections.GetChildConnection(type, parent, i);
            if(Objects().GetByUid<T>(c->child_uid))
                counter++;
        }
        return counter;
    }
    template<typename T>
    size_t CountParents(CONNECTION_TYPE type, int64_t child) {
        size_t sz = connections.CountParents(type, child);
        size_t counter = 0;
        for(size_t i = 0; i < sz; ++i) {
            Connection* c = 
                connections.GetParentConnection(type, child, i);
            if(Objects().GetByUid<T>(c->parent_uid))
                counter++;
        }
        return counter;
    }
    template<typename T>
    T* GetChild(CONNECTION_TYPE type, int64_t parent) {
        std::string name;
        return GetChild<T>(type, parent, 0, name);
    }
    template<typename T>
    T* GetParent(CONNECTION_TYPE type, int64_t child) {
        std::string name;
        return GetParent<T>(type, child, 0, name);
    }
    template<typename T>
    T* GetChild(CONNECTION_TYPE type, int64_t parent, std::string& name) {
        return GetChild<T>(type, parent, 0, name);
    }
    template<typename T>
    T* GetParent(CONNECTION_TYPE type, int64_t child, std::string& name) {
        return GetParent<T>(type, child, 0, name);
    }
    template<typename T>
    T* GetChild(CONNECTION_TYPE type, int64_t parent, size_t id) {
        std::string name;
        return GetChild<T>(type, parent, id, name);
    }
    template<typename T>
    T* GetParent(CONNECTION_TYPE type, int64_t child, size_t id) {
        std::string name;
        return GetParent<T>(type, child, id, name);
    }
    template<typename T>
    T* GetChild(CONNECTION_TYPE type, int64_t parent, size_t id, std::string& name) {
        size_t sz = connections.CountChildren(type, parent);
        size_t counter = 0;
        for(size_t i = 0; i < sz; ++i) {
            Connection* c = 
                connections.GetChildConnection(type, parent, i);
            T* o;
            if(o = Objects().GetByUid<T>(c->child_uid)) {
                if(counter == id) {
                    name = c->name;
                    return o;
                }
                counter++;
            }
        }
        return 0;
    }
    template<typename T>
    T* GetParent(CONNECTION_TYPE type, int64_t child, size_t id, std::string& name) {
        size_t sz = connections.CountParents(type, child);
        size_t counter = 0;
        for(size_t i = 0; i < sz; ++i) {
            Connection* c = 
                connections.GetParentConnection(type, child, i);
            T* o;
            if(o = Objects().GetByUid<T>(c->parent_uid)) {
                if(counter == id) {
                    name = c->name;
                    return o;
                }
                counter++;
            }
        }
        return 0;
    }
private:
    ObjectContainer objects;
    Properties* properties;
    ConnectionContainer connections;
    std::vector<Model*> rootModels;

    void Finalize(Node& node);
};

}

#endif
