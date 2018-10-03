#ifndef FBX_POSE_H
#define FBX_POSE_H

#include "../fbx_/fbx_math.h"
#include "fbx_node.h"
#include "fbx_object.h"

class FbxPose : public FbxObject {
public:
    virtual const char* Type() const { return "Pose"; }
    virtual bool Make(FbxNode& node);

    FbxMatrix4 GetPose(int64_t uid) { 
        auto it = transforms.find(uid);
        if(it == transforms.end()) {
            return FbxMatrix4(1.0f);
        }
        return it->second;
    }

    std::string subtype;
    std::map<int64_t, FbxMatrix4> transforms;
};

#endif
