#ifndef FBX_MODEL_2_H
#define FBX_MODEL_2_H

#include "fbx_object.h"
#include "fbx_log.h"
#include "fbx_macro.h"
#include "fbx_properties.h"

namespace Fbx {

class Model : public Object {
public:
    enum Type {
        Unknown,
        Camera,
        CameraStereo,
        CameraSwitcher,
        Light,
        Mesh,
        Nurbs,
        NurbsCurve,
        NurbsSurface,
        TrimNurbsSurface,
        Patch,
        LodGroup,
        Marker,
        Skeleton,
        LimbNode = Skeleton,
        Null
    };

    size_t ChildCount() const;
    Model* GetChild(size_t i);

    virtual bool Make(Node& node) {
        type = node.GetProperty(2).GetString();
        lclTranslation = FbxDVector3(0.0f, 0.0f, 0.0f);
        lclRotation = FbxDVector3(0.0f, 0.0f, 0.0f);
        lclScaling = FbxDVector3(1.0f, 1.0f, 1.0f);
        preRotation = FbxDVector3(0.0f, 0.0f, 0.0f);
        postRotation = FbxDVector3(0.0f, 0.0f, 0.0f);

        Properties* props = node.ConvertChild<Properties>();
        if(!props) {
            Log("Failed to get model properties");
            return false;
        }     
        if(props->Get("Lcl Translation")) {
            lclTranslation =
                props->GetValue<FbxDVector3>("Lcl Translation");
        }
        if(props->Get("Lcl Rotation")) {
            lclRotation =
                props->GetValue<FbxDVector3>("Lcl Rotation") * FbxPi / 180.0f;
        }
        if(props->Get("Lcl Scaling")) {
            lclScaling =
                props->GetValue<FbxDVector3>("Lcl Scaling");
        }
        if(props->Get("PreRotation")) {
            preRotation = 
                props->GetValue<FbxDVector3>("PreRotation") * FbxPi / 180.0f;
        }
        if(props->Get("PostRotation")) {
            postRotation =
                props->GetValue<FbxDVector3>("PostRotation") * FbxPi / 180.0f;
        }

        FbxQuat preQuat = FbxEulerToQuat(preRotation);
        FbxQuat postQuat = FbxEulerToQuat(postRotation);
        FbxQuat rotation = FbxEulerToQuat(lclRotation);
        lclTransform = 
            FbxTranslate(FbxMatrix4(1.0f), lclTranslation) *
            FbxToMatrix4(preQuat * rotation * postQuat) *
            FbxScale(FbxMatrix4(1.0f), lclScaling);

        return true;
    }

    static bool IdentifyNode(Node& node) {
        if(node.GetName() != "Model"
            || node.PropCount() < 3
            || !node.GetProperty(0).IsInt64()
            || !node.GetProperty(1).IsString()
            || !node.GetProperty(2).IsString()) 
            return false;
        return true;
    }

    std::string type;

    FbxDVector3 preRotation;
    FbxDVector3 postRotation;
    FbxDVector3 lclTranslation;
    FbxDVector3 lclRotation;
    FbxDVector3 lclScaling;

    FbxMatrix4 lclTransform;
};

}

#endif
