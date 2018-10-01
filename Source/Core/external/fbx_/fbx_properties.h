#ifndef FBX_PROPERTIES_2_H
#define FBX_PROPERTIES_2_H

#include "fbx_object.h"
#include "fbx_log.h"
#include "fbx_macro.h"

namespace Fbx {

class Properties : public Object {
public:
    virtual bool Make(Node& node) {
        // TODO

        return true;
    }

    static bool IdentifyNode(Node& node) {
        if(node.GetName() != "Properties70") 
            return false;
        return true;
    }
};

}

#endif