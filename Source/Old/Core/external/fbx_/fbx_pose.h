#ifndef FBX_POSE_2_H
#define FBX_POSE_2_H

#include "fbx_object.h"
#include "fbx_log.h"
#include "fbx_macro.h"

namespace Fbx {

class Pose : public Object {
public:
    virtual bool Make(Node& node) {
        return true;
    }

    static bool IdentifyNode(Node& node) {
        if(node.GetName() != "Pose"
            || node.PropCount() < 3
            || !node.GetProperty(0).IsInt64()
            || !node.GetProperty(1).IsString()
            || !node.GetProperty(2).IsString()) 
            return false;
        return true;
    }
};

}

#endif
