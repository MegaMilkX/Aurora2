#ifndef FBX_ANIMATION_STACK_H
#define FBX_ANIMATION_STACK_H

#include "fbx_node.h"
#include "fbx_animation_layer.h"
#include "fbx_connections.h"

#include "fbx_object.h"

class FbxAnimationStack : public FbxObject
{
public:
    const std::string& Name() const { return name; }
    size_t LayerCount() const { return layers.size(); }
    FbxAnimationLayer* GetLayer(int i) { return layers[i]; }

    virtual const char* Type() const { return "AnimationStack"; }
    virtual bool Make(FbxNode& node);
private:
    std::string name;
    double fps;
    double length;
    std::vector<FbxAnimationLayer*> layers;
};

#endif
