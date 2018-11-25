#include "fbx_animation_layer.h"
#include "fbx_scene.h"

bool FbxAnimationLayer::Make(FbxNode& node)
{
    if(node.GetName() != Type()) {
        return false;
    }
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

FbxVector3 FbxAnimationLayer::EvalTranslation(FbxModel& model, double frame) {
    int64_t time = frame * (FbxTimeSecond / scene->Settings().frameRate);
    FbxAnimationCurveNode* curveNode = 
        FindCurveNode(model.GetName(), "T");
    if(!curveNode) return model.GetLclTranslation();

    FbxAnimationCurve* cx = curveNode->GetCurve("d|X");
    FbxAnimationCurve* cy = curveNode->GetCurve("d|Y");
    FbxAnimationCurve* cz = curveNode->GetCurve("d|Z");

    FbxVector3 r(model.GetLclTranslation());

    if(cx) r.x = cx->Evaluate(time);
    if(cy) r.y = cy->Evaluate(time);
    if(cz) r.z = cz->Evaluate(time);
    return r;
}
FbxQuat FbxAnimationLayer::EvalRotation(FbxModel& model, double frame) {
    int64_t time = frame * (FbxTimeSecond / scene->Settings().frameRate);
    FbxAnimationCurveNode* curveNode = 
        FindCurveNode(model.GetName(), "R");
    if(!curveNode) return FbxEulerToQuat(model.GetLclRotation());

    FbxAnimationCurve* cx = curveNode->GetCurve("d|X");
    FbxAnimationCurve* cy = curveNode->GetCurve("d|Y");
    FbxAnimationCurve* cz = curveNode->GetCurve("d|Z");

    FbxVector3 r(model.GetLclRotation());

    if(cx) r.x = cx->Evaluate(time);
    if(cy) r.y = cy->Evaluate(time);
    if(cz) r.z = cz->Evaluate(time);

    FbxQuat q = FbxEulerToQuat(r);
    FbxQuat pre = FbxEulerToQuat(model.GetPreRotation());
    FbxQuat post = FbxEulerToQuat(model.GetPostRotation());

    return pre * q * post;
}
FbxVector3 FbxAnimationLayer::EvalScale(FbxModel& model, double frame) {
    int64_t time = frame * (FbxTimeSecond / scene->Settings().frameRate);
    FbxAnimationCurveNode* curveNode = FindCurveNode(
        model.GetName(),
        "S"
    );
    if(!curveNode) return model.GetLclScaling();

    FbxAnimationCurve* cx = curveNode->GetCurve("d|X");
    FbxAnimationCurve* cy = curveNode->GetCurve("d|Y");
    FbxAnimationCurve* cz = curveNode->GetCurve("d|Z");

    FbxVector3 r(model.GetLclScaling());

    if(cx) r.x = cx->Evaluate(time);
    if(cy) r.y = cy->Evaluate(time);
    if(cz) r.z = cz->Evaluate(time);
    return r;
}

FbxAnimationCurveNode* FbxAnimationLayer::FindCurveNode(const std::string& tgt_model, const std::string& name) {
    FbxAnimationCurveNode* curveNode = 0;

    for(auto cn : curveNodes) {
        if(cn->Name() == name && cn->OwnerName() == tgt_model) {
            curveNode = cn;
            break;
        }
    }

    return curveNode;
}