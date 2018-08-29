#include "fbx_animation_curve_node.h"
#include "fbx_scene.h"

bool FbxAnimationCurveNode::Make(FbxNode& node) {
    uid = node.GetProperty(0).GetInt64();
    name = node.GetProperty(1).GetString();

    size_t child_count = scene->Connections().CountChildrenOP(uid);
    for(size_t i = 0; i < child_count; ++i)
    {
        FbxConnection* conn = scene->Connections().GetChildOPConnection(uid, i);
        int64_t child_uid = conn->child_uid;
        FbxAnimationCurve* curve =
            scene->GetByUid<FbxAnimationCurve>(child_uid);
        if(!curve) continue;
        curve->SetName(conn->name);
        curves.emplace_back(curve);
    }

    return true;
}