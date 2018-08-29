#ifndef FBX_ANIMATION_CURVE_NODE_H
#define FBX_ANIMATION_CURVE_NODE_H

#include "fbx_object.h"
#include "fbx_animation_curve.h"

class FbxAnimationCurveNode : public FbxObject
{
public:
    virtual const char* Type() const { return "AnimationCurveNode"; }
    virtual bool Make(FbxNode& node);

    void SetName(const std::string& name) { this->name = name; }
    const std::string& Name() const { return name; }
    const std::string& OwnerName() const { return ownerName; }

    size_t CurveCount() const { return curves.size(); }
    FbxAnimationCurve* GetCurve(size_t i) { return curves[i]; }
private:
    std::string name;
    std::string ownerName;
    std::vector<FbxAnimationCurve*> curves;
};

#endif
