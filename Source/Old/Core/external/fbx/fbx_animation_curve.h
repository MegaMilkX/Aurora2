#ifndef FBX_ANIMATION_CURVE_H
#define FBX_ANIMATION_CURVE_H

#include "fbx_object.h"

struct FbxKeyframe
{
    FbxKeyframe(int64_t frame, float value)
    : frame(frame), value(value) {}
    bool operator<(const FbxKeyframe& other) const
    { return frame < other.frame; }
    int64_t frame;
    float value;
};

class FbxAnimationCurve : public FbxObject
{
public:
    virtual const char* Type() const { return "AnimationCurve"; }
    virtual bool Make(FbxNode& node);

    void SetName(const std::string& name) { this->name = name; }
    const std::string& Name() const { return name; }

    float Evaluate(int64_t time) {
        FbxKeyframe* k0 = 0, *k1 = 0; 
        for(int i = keyframes.size() - 1; i >= 0; --i) {
            k0 = &keyframes[i];
            if(i == keyframes.size() - 1)
                k1 = k0;
            else
                k1 = &keyframes[i + 1];
            if(k0->frame <= time)
                break;
        }
        if(k0 != k1) {
            float a = k0->value;
            float b = k1->value;
            float t = (float)(time - k0->frame) / (float)(k1->frame - k0->frame);
            
            return a + t * (b - a);
        }
        else if(k0) {
            return k0->value;
        }
        else {
            return defaultValue;
        }
    }

    unsigned KeyframeCount() { return keyframes.size(); }
    FbxKeyframe* GetKeyframe(unsigned id) { return &keyframes[id]; }
    const std::vector<FbxKeyframe>& GetKeyframes() { return keyframes; }
private:
    std::string name;
    std::vector<FbxKeyframe> keyframes;
    float defaultValue;
};

#endif
