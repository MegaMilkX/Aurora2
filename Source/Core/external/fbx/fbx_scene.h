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

enum FBX_TIME_MODE
{
    FBX_FRAMES_DEFAULT,
    FBX_FRAMES_120,
    FBX_FRAMES_100,
    FBX_FRAMES_60,
    FBX_FRAMES_50,
    FBX_FRAMES_48,
    FBX_FRAMES_30,
    FBX_FRAMES_30_DROP,
    FBX_FRAMES_NTSC_DROP,
    FBX_FRAMES_NTSC_FULL,
    FBX_FRAMES_PAL,
    FBX_FRAMES_CINEMA,
    FBX_FRAMES_1000m,
    FBX_FRAMES_CINEMA_ND,
    FBX_FRAMES_CUSTOM,
    FBX_FRAMES_96,
    FBX_FRAMES_72,
    FBX_FRAMES_59dot94,
    FBX_FRAMES_MODE_COUNT
};

struct FbxSettings
{
    double scaleFactor = 1.0;
    double frameRate = 60.0;
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

    template<typename T>
    T* _make(FbxNode& node);
    template<typename T>
    T* _make(int64_t uid);

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
    if(container.objects.count(uid) == 0)
    {
        return _make<T>(uid);
    }    

    return (T*)container.objects[uid].get();
}
template<typename T>
T* FbxScene::_make(FbxNode& node) {
    auto& container = objects[FbxTypeInfo<T>::Index()];
    int64_t uid = node.GetProperty(0).GetInt64();
    T* o = new T();
    o->SetScene(this);
    container.objects[uid].reset(o);
    container.uids.emplace_back(uid);
    if(!o->Make(node)) {
        delete o;
        return 0;
    }
    return o;
}
template<typename T>
T* FbxScene::_make(int64_t uid) {
    for(unsigned i = 0; i < rootNode.ChildCount(T().Type()); ++i) {
        FbxNode& node = rootNode.GetNode(T().Type(), i);
        if(!node.GetProperty(0).IsInt64()) return 0;
        if(node.GetProperty(0).GetInt64() == uid) return _make<T>(node);
    }
    return 0;
}

#endif
