#ifndef ANIMATION_LAYER_H
#define ANIMATION_LAYER_H

#include "fbx_node.h"

#include "fbx_object.h"

class FbxAnimationLayer : public FbxObject
{
public:
    virtual bool Make(FbxNode& node)
    {
        name = node.GetProperty(1).GetString();
        return true;
    }
private:
    std::string name;
};

#endif
