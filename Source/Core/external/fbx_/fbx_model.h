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

    virtual bool Make(Node& node) {
        type = node.GetProperty(2).GetString();
        lclTranslation = FbxVector3(0.0f, 0.0f, 0.0f);
        lclRotation = FbxVector3(0.0f, 0.0f, 0.0f);
        lclScaling = FbxVector3(1.0f, 1.0f, 1.0f);
        preRotation = FbxVector3(0.0f, 0.0f, 0.0f);
        postRotation = FbxVector3(0.0f, 0.0f, 0.0f);

        Properties* props = node.ConvertChild<Properties>();
        if(!props) {
            Log("Failed to get model properties");
            return false;
        }
        
        // TODO

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

    FbxVector3 preRotation;
    FbxVector3 postRotation;
    FbxVector3 lclTranslation;
    FbxVector3 lclRotation;
    FbxVector3 lclScaling;

    FbxMatrix4 lclTransform;
};

}

#endif
