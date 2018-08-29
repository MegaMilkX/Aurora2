#include "fbx_animation_layer.h"
#include "fbx_scene.h"

bool FbxAnimationLayer::Make(FbxNode& node)
{
    int64_t uid = node.GetProperty(0).GetInt64();
    name = node.GetProperty(1).GetString();

    size_t child_count = scene->Connections().CountChildren(FBX_OBJECT_OBJECT, uid);
    for(size_t i = 0; i < child_count; ++i)
    {
        int64_t child_uid = scene->Connections().GetChild(FBX_OBJECT_OBJECT, uid, i);
        FbxAnimationCurveNode* curveNode =
            scene->GetByUid<FbxAnimationCurveNode>(child_uid);
        if(!curveNode) continue;

        curveNodes.emplace_back(curveNode);
    }

    return true;
}