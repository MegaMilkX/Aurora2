#ifndef ANIMATION_LAYER_H
#define ANIMATION_LAYER_H

#include "fbx_node.h"

#include "fbx_object.h"

#include "fbx_animation_curve_node.h"

#include "fbx_math.h"
#include "fbx_model.h"

class FbxAnimationLayer : public FbxObject
{
public:
    virtual const char* Type() const { return "AnimationLayer"; }
    virtual bool Make(FbxNode& node);

    FbxVector3 EvalTranslation(FbxModel& model, double frame);
    FbxQuat EvalRotation(FbxModel& model, double frame);
    FbxVector3 EvalScale(FbxModel& model, double frame);

    FbxAnimationCurveNode* FindCurveNode(const std::string& tgt_model, const std::string& name);

    size_t CurveNodeCount() const { return curveNodes.size(); }
    FbxAnimationCurveNode* GetCurveNode(size_t i) { return curveNodes[i]; }
private:
    std::string name;
    std::vector<FbxAnimationCurveNode*> curveNodes;
};

#endif
