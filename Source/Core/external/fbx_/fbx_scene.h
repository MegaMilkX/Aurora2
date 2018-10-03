#ifndef FBX_SCENE_2_H
#define FBX_SCENE_2_H

#include <stdint.h>
#include <algorithm>
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

    void _dumpFile(const std::string& filename)
    {
        std::ostringstream sstr;
        rootNode.Print(sstr);
        std::ofstream f(filename + ".dump", std::ios::out);
        f << sstr.str();
        f.close();
    }
private:
    ObjectContainer objects;
    Properties* properties;
    ConnectionContainer connections;
    std::vector<Model*> rootModels;

    Node rootNode;
    void Finalize(Node& node);
};

inline bool MakeBlendIndicesAndWeights(
    Scene& scene, 
    DeformerSkin* skin, 
    Geometry* geom, 
    IndexedTriangleMesh& triMesh,
    std::vector<FbxVector4>& indices,
    std::vector<FbxVector4>& weights
) {
    // CP - control point (a unique vertex in fbx terms)
    std::vector<FbxVector4> indicesPerCP(geom->controlPoints.size());
    std::fill(indicesPerCP.begin(), indicesPerCP.end(), FbxVector4(-1.0f, -1.0f, -1.0f, -1.0f));
    std::vector<FbxVector4> weightsPerCP(geom->controlPoints.size());

    struct Bone {
        Model* model;
        DeformerCluster* cluster;
    };
    std::vector<Bone> bones;
    size_t cluster_count = 
        scene.CountChildren<Fbx::DeformerCluster>(Fbx::OBJECT_OBJECT, skin->GetUid());
    for(size_t j = 0; j < cluster_count; ++j) {
        Fbx::DeformerCluster* cluster = 
            scene.GetChild<Fbx::DeformerCluster>(Fbx::OBJECT_OBJECT, skin->GetUid());
        if(cluster->indices.size() != cluster->weights.size()) {
            FBX_LOGW("Cluster " << cluster->GetUid() << ": index and weight count mismatch: "
                << cluster->indices.size() << " vs " << cluster->weights.size());
            return false;
        }
        Fbx::Model* model =
            scene.GetChild<Fbx::Model>(Fbx::OBJECT_OBJECT, cluster->GetUid());
        if(!model) {
            FBX_LOGW("Failed to get model for deformer " << cluster->GetUid());
            return false;
        }
        bones.emplace_back(Bone{ model, cluster });
    }
    std::sort(bones.begin(), bones.end(), [&](const Bone& a, const Bone& b) ->bool {
        return a.model->GetName() < b.model->GetName();
    });

    /*

    for(size_t i = 0; i < bones.size(); ++i) {
        Bone& b = bones[i];
        float boneIndex = (float)i;
        for(size_t j = 0; j < b.cluster->indices.size(); ++j) {
            int32_t controlPoint = b.cluster->indices[j];
            double weight = b.cluster->weights[j];
            for(size_t k = 0; k < 4; ++k) {
                if(indicesPerCP[controlPoint][k] < 0.0f) {
                    indicesPerCP[controlPoint][k] = boneIndex;
                    weightsPerCP[controlPoint][k] = (float)weight;
                    break;
                }
            }
        }
    }
    for(auto& ivec4 : indicesPerCP) {
        for(size_t i = 0; i < 4; ++i) {
            if(ivec4[i] < 0.0f) {
                ivec4[i] = 0.0f;
            }
        }
    }

    for(size_t i = 0; i < triMesh.controlPointReferences.size(); ++i) {
        int32_t cp = triMesh.controlPointReferences[i];
        indices.emplace_back(indicesPerCP[cp]);
        weights.emplace_back(weightsPerCP[cp]);
    }

    */

    return true;
}

} // Fbx

#endif
