#include "fbx_animation_curve_node.h"
#include "fbx_scene.h"

bool FbxAnimationCurveNode::Make(FbxNode& node) {
    if(node.GetName() != Type()) {
        return false;
    }
    uid = node.GetProperty(0).GetInt64();
    name = node.GetProperty(1).GetString();

    size_t child_count = scene->Connections().CountChildren(FBX_OBJECT_PROPERTY, uid);
    for(size_t i = 0; i < child_count; ++i)
    {
        FbxConnection* conn = scene->Connections().GetChildConnection(FBX_OBJECT_PROPERTY, uid, i);
        int64_t child_uid = conn->child_uid;
        FbxAnimationCurve* curve =
            scene->GetByUid<FbxAnimationCurve>(child_uid);
        if(!curve) continue;
        curve->SetName(conn->name);
        curves.emplace_back(curve);
    }

    int64_t propOwner = scene->Connections().GetParent(FBX_OBJECT_PROPERTY, uid, 0);
    if(propOwner < 0) { return false; }

    FbxModel* model = scene->GetByUid<FbxModel>(propOwner);
    if(!model) return false;

    ownerName = model->GetName();

    return true;
}