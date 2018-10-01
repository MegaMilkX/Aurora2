#ifndef FBX_DEFORMER_CLUSTER_2_H
#define FBX_DEFORMER_CLUSTER_2_H

#include "fbx_object.h"
#include "fbx_log.h"
#include "fbx_macro.h"

namespace Fbx {

class DeformerCluster : public Object {
public:
    virtual bool Make(Node& node) {
        return true;
    }

    static bool IdentifyNode(Node& node) {
        if(node.GetName() != "Deformer"
            || node.PropCount() < 3
            || !node.GetProperty(0).IsInt64()
            || !node.GetProperty(1).IsString()
            || !node.GetProperty(2).IsString()
            || node.GetProperty(2).GetString() != "Cluster") 
            return false;
        return true;
    }
};

}

#endif