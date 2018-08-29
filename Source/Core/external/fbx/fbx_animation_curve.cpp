#include "fbx_animation_curve.h"

#include <algorithm>

bool FbxAnimationCurve::Make(FbxNode& node) {
    std::vector<int64_t> keyTime = 
        node.GetNode("KeyTime", 0).GetProperty(0).GetArray<int64_t>();
    std::vector<float> keyValue =
        node.GetNode("KeyValueFloat", 0).GetProperty(0).GetArray<float>();
    defaultValue = node.GetNode("Default", 0).GetProperty(0).GetFloat();

    for(size_t i = 0; i < keyTime.size() && i < keyValue.size(); ++i) {
        keyframes.emplace_back(FbxKeyframe(keyTime[i], keyValue[i]));
    }

    std::sort(keyframes.begin(), keyframes.end());

    return true;
}