#ifndef FBX_POSE_H
#define FBX_POSE_H

#include "fbx_math.h"
#include "fbx_node.h"
#include "fbx_object.h"

class FbxPose : public FbxObject {
public:
    virtual const char* Type() const { return "Pose"; }
    virtual bool Make(FbxNode& node);

    std::string subtype;
    std::map<int64_t, FbxMatrix4> transforms;
};

#endif
