#ifndef FBX_SCENE_H
#define FBX_SCENE_H

#include "fbx_node.h"
#include "fbx_model.h"
#include "fbx_mesh.h"
#include "fbx_light.h"
#include "fbx_geometry.h"
#include "fbx_animation_stack.h"
#include "fbx_connections.h"
#include "fbx_math.h"
#include "fbx_type_index.h"

#include <string>
#include <vector>
#include <iostream>
#include <memory>

struct FbxSettings
{
    double scaleFactor = 1.0;
};

class FbxScene
{
private:
    FbxScene(){}
public:
    static FbxScene* Create() { return new FbxScene(); }
    void Destroy() { delete this; }

    FbxSettings& Settings() { return settings; }
    FbxConnections& Connections() { return connections; }

    template<typename T>
    size_t Count();
    template<typename T>
    T* Get(size_t i);
    template<typename T>
    T* GetByUid(int64_t uid);
    template<typename T>
    T* GetOrCreateByUid(int64_t uid);
    template<typename T>
    T* Make(FbxNode& node);

    size_t RootModelCount() const { return rootModels.size(); }
    FbxModel* GetRootModel(size_t i) { return GetByUid<FbxModel>(rootModels[i]); }

    void _dumpFile(const std::string& filename);
    /* Don't use this */
    void _finalize();
    /* This is not a spatial node, it's a file node */
    FbxNode& _getRootNode() { return rootNode; }
private:
    void _makeGlobalSettings();
    FbxModel* _makeModel(FbxNode& node);
    void _makeMesh(FbxNode& node);

    FbxNode rootNode;

    FbxSettings settings;

    struct FbxObjectContainer
    {
        std::vector<int64_t> uids;
        std::map<int64_t, std::shared_ptr<FbxObject>> objects;
    };
    std::map<FbxTypeIndex, FbxObjectContainer> objects;

    std::vector<int64_t> rootModels;

    FbxConnections connections;
};

template<typename T>
size_t FbxScene::Count() {
    return objects[FbxTypeInfo<T>::Index()].uids.size();
}
template<typename T>
T* FbxScene::Get(size_t i) {
    auto& container = objects[FbxTypeInfo<T>::Index()];
    return (T*)container.objects[container.uids[i]].get();
}
template<typename T>
T* FbxScene::GetByUid(int64_t uid) {
    auto& container = objects[FbxTypeInfo<T>::Index()];
    return (T*)container.objects[uid].get();
}
template<typename T>
T* FbxScene::GetOrCreateByUid(int64_t uid) {
    auto& container = objects[FbxTypeInfo<T>::Index()];
    auto it = container.objects.find(uid);
    if(it == container.objects.end())
    {
        T* o = new T();
        o->SetScene(this);
        container.objects[uid].reset(o);
        container.uids.emplace_back(uid);
        return o;
    }
    else
    {
        return (T*)it->second.get();
    }
}
template<typename T>
T* FbxScene::Make(FbxNode& node) {
    auto& container = objects[FbxTypeInfo<T>::Index()];
    int64_t uid = node.GetProperty(0).GetInt64();
    T* o = new T();
    o->SetScene(this);
    if(!o->Make(node)) {
        delete o;
        return 0;
    }

    container.objects[uid].reset(o);
    container.uids.emplace_back(uid);
    return o;
}

#endif
