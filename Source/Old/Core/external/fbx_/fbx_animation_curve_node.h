#ifndef FBX_ANIM_CURVE_NODE_2_H
#define FBX_ANIM_CURVE_NODE_2_H

#include "fbx_object.h"
#include "fbx_log.h"
#include "fbx_macro.h"

namespace Fbx {

class AnimationCurveNode : public Object {
public:
    virtual bool Make(Node& node) {
        return true;
    }

    static bool IdentifyNode(Node& node) {
        if(node.GetName() != "AnimationCurveNode"
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