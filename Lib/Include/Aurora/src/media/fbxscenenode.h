#ifndef AU_FBXSCENENODE_H
#define AU_FBXSCENENODE_H

#include "fbxnode.h"
#include "fbxutil.h"

namespace Au{
namespace Media{
namespace FBX{

class SceneNode
{
public:
    enum ANIM_MODE
    {
        NOT_ANIMATED,
        ANIMATED,
        ANIMATED_ADD
    };
    SceneNode(): uid(0), source(0) {}
    SceneNode(Settings& settings, Node& n)
    {
        _settings = &settings;
        
        uid = n[0].GetInt64();
        name = n[1].GetString();
        type = n[2].GetString();
        source = &n;
        
        _getLclTranslation();
        _getLclRotation();
        _getLclScaling();
        _getPreRotation();
        _getPostRotation();
    }
    
    int64_t UID() { return uid; }
    std::string Name() { return name; }
    std::string Type() { return type; }
    Node* SourceNode() { return source; }
    
    Au::Math::Vec3f LclTranslation() { return lclTranslation; }
    ANIM_MODE LclTranslationAnimMode() { return lclTranslationAnimMode; }
    Au::Math::Vec3f LclRotation() { return lclRotation; }
    ANIM_MODE LclRotationAnimMode() { return lclRotationAnimMode; }
    Au::Math::Vec3f LclScaling() { return lclScaling; }
    ANIM_MODE LclScalingAnimMode() { return lclScalingAnimMode; }
    
    Au::Math::Vec3f PreRotation()
    {
        return preRotation;
    }
    Au::Math::Quat PreRotationQuat()
    {
        return Au::Math::EulerToQuat(preRotation);
    }
    
    Au::Math::Vec3f PostRotation()
    {
        return postRotation;
    }
    Au::Math::Quat PostRotationQuat()
    {
        return Au::Math::EulerToQuat(postRotation);
    }
private:
    void _getAnimMode(Node& n, ANIM_MODE& mode)
    {
        std::string animMode = n[3].GetString();
        if(animMode == "A")
            mode = ANIMATED;
        else if(animMode == "A+")
            mode = ANIMATED_ADD;
        else
            mode = NOT_ANIMATED;
    }

    void _getLclTranslation()
    {
        lclTranslation = Au::Math::Vec3f(0.0f, 0.0f, 0.0f);
        Node* ptr = source->Get("Properties70").GetWhere(0, "Lcl Translation");
        if(!ptr)
            return;
        Node& n = *ptr;
        lclTranslation.x = (float)n[4].GetDouble() * (float)_settings->scaleFactor;
        lclTranslation.y = (float)n[5].GetDouble() * (float)_settings->scaleFactor;
        lclTranslation.z = (float)n[6].GetDouble() * (float)_settings->scaleFactor;
        
        //ConvertVector(*_settings, lclTranslation);
        
        _getAnimMode(n, lclTranslationAnimMode);
    }
    
    void _getLclRotation()
    {
        lclRotation = Au::Math::Vec3f(0.0f, 0.0f, 0.0f);
        Node* ptr = source->Get("Properties70").GetWhere(0, "Lcl Rotation");
        if(!ptr)
            return;
        Node& n = *ptr;
        lclRotation.x = ((float)n[4].GetDouble() * Au::Math::PI) / 180.0f;
        lclRotation.y = ((float)n[5].GetDouble() * Au::Math::PI) / 180.0f;
        lclRotation.z = ((float)n[6].GetDouble() * Au::Math::PI) / 180.0f;
        
        //ConvertVector(*_settings, lclRotation, false);
        
        _getAnimMode(n, lclRotationAnimMode);
    }
    
    void _getLclScaling()
    {
        lclScaling = Au::Math::Vec3f(1.0f, 1.0f, 1.0f);
        Node* ptr = source->Get("Properties70").GetWhere(0, "Lcl Scaling");
        if(!ptr)
            return;
        Node& n = *ptr;
        lclScaling.x = (float)n[4].GetDouble();
        lclScaling.y = (float)n[5].GetDouble();
        lclScaling.z = (float)n[6].GetDouble();
        
        //ConvertVector(*_settings, lclScaling, false);
        
        _getAnimMode(n, lclScalingAnimMode);
    }

    void _getPreRotation()
    {
        preRotation = Au::Math::Vec3f(0.0f, 0.0f, 0.0f);
        Node* ptr = source->Get("Properties70").GetWhere(0, "PreRotation");
        if(!ptr)
            return;
        Node& n = *ptr;
        preRotation.x = ((float)n[4].GetDouble() * Au::Math::PI) / 180.0f;
        preRotation.y = ((float)n[5].GetDouble() * Au::Math::PI) / 180.0f;
        preRotation.z = ((float)n[6].GetDouble() * Au::Math::PI) / 180.0f;
        
        //ConvertVector(*_settings, preRotation);
    }
    void _getPostRotation()
    {
        postRotation = Au::Math::Vec3f(0.0f, 0.0f, 0.0f);
        Node* ptr = source->Get("Properties70").GetWhere(0, "PostRotation");
        if(!ptr)
            return;
        Node& n = *ptr;
        postRotation.x = ((float)n[4].GetDouble() * Au::Math::PI) / 180.0f;
        postRotation.y = ((float)n[5].GetDouble() * Au::Math::PI) / 180.0f;
        postRotation.z = ((float)n[6].GetDouble() * Au::Math::PI) / 180.0f;
        
        //ConvertVector(*_settings, postRotation);
    }

    int64_t uid;
    std::string name;
    std::string type;
    
    Au::Math::Vec3f lclTranslation;
    ANIM_MODE lclTranslationAnimMode;
    Au::Math::Vec3f lclRotation;
    ANIM_MODE lclRotationAnimMode;
    Au::Math::Vec3f lclScaling;
    ANIM_MODE lclScalingAnimMode;
    
    Au::Math::Vec3f preRotation;
    Au::Math::Vec3f postRotation;
    
    Node* source;
    
    Settings* _settings;
};

}
}
}

#endif
