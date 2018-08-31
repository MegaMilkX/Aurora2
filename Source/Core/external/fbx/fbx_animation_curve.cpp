#include "fbx_animation_curve.h"
#include "fbx_scene.h"
#include "fbx_animation_curve_node.h"
#include <algorithm>

bool FbxAnimationCurve::Make(FbxNode& node) {
    int64_t uid = node.GetProperty(0).GetInt64();

    std::vector<int64_t> keyTime = 
        node.GetNode("KeyTime", 0).GetProperty(0).GetArray<int64_t>();
    std::vector<float> keyValue =
        node.GetNode("KeyValueFloat", 0).GetProperty(0).GetArray<float>();
    defaultValue = node.GetNode("Default", 0).GetProperty(0).GetFloat();

    int64_t parent_uid = scene->Connections().GetParent(FBX_OBJECT_PROPERTY, uid, 0);
    if(parent_uid <= 0) return false;
    FbxAnimationCurveNode* curveNode = scene->GetByUid<FbxAnimationCurveNode>(parent_uid);
    if(!curveNode) return false;
    if(curveNode->Name() == "T") {
        for(auto& v : keyValue) {
            v *= scene->Settings().scaleFactor;
        }
    }
    else if(curveNode->Name() == "R") {
        for(auto& v : keyValue) {
            v *= FbxPi / 180.0f;
        }
    }

    for(size_t i = 0; i < keyTime.size() && i < keyValue.size(); ++i) {
        keyframes.emplace_back(FbxKeyframe(keyTime[i], keyValue[i]));
    }

    std::sort(keyframes.begin(), keyframes.end());

    return true;
}