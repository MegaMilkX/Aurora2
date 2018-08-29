#include "fbx_animation_stack.h"
#include "fbx_scene.h"

const int64_t FbxTimeSecond = 46186158000;

bool FbxAnimationStack::Make(FbxNode& node) {
    if(!scene) return false;
    auto& conns = scene->Connections();
    int64_t uid = node.GetProperty(0).GetInt64();
    name = node.GetProperty(1).GetString();
    for(unsigned i = 0; i < conns.CountChildrenOO(uid); ++i) {
        int64_t animLayerUid = conns.GetChildOO(uid, i);
        auto layer = scene->GetByUid<FbxAnimationLayer>(animLayerUid);
        if(!layer) continue;
        layers.emplace_back(layer);
    }

    int64_t start = 0;
    int64_t end = 0;

    if(!node.ChildCount("Properties70"))
        return false;

    FbxNode& props = node.GetNode("Properties70", 0);
    for(unsigned i = 0; i < props.ChildCount("P"); ++i) {
        FbxNode& p = props.GetNode("P", i);
        if(p.GetProperty(0).GetString() == "LocalStart") {
            start = p.GetProperty(4).GetInt64();
        } else if(p.GetProperty(0).GetString() == "LocalStop") {
            end = p.GetProperty(4).GetInt64();
        }
    }

    double timePerFrame = FbxTimeSecond / scene->Settings().frameRate;
    length = (end - start) / timePerFrame;
    fps = scene->Settings().frameRate;

    return true;
}