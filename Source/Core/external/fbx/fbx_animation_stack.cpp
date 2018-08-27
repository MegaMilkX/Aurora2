#include "fbx_animation_stack.h"
#include "fbx_scene.h"

bool FbxAnimationStack::Make(FbxNode& node) {
    if(!scene) return false;
    auto& conns = scene->Connections();
    int64_t uid = node.GetProperty(0).GetInt64();
    name = node.GetProperty(1).GetString();
    for(unsigned i = 0; i < conns.CountChildrenOO(uid); ++i) {
        int64_t animLayerUid = conns.GetChildOO(uid, i);
        layers.emplace_back(animLayerUid);
    }
    return true;
}