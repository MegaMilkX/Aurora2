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
    int64_t GetLayerUid(int i) { return layers[i]; }

    void Make(FbxNode& node, FbxConnections& conns)
    {
        int64_t uid = node.GetProperty(0).GetInt64();
        name = node.GetProperty(1).GetString();
        for(unsigned i = 0; i < conns.CountChildrenOO(uid); ++i) {
            int64_t animLayerUid = conns.GetChildOO(uid, i);
            layers.emplace_back(animLayerUid);
        }
    }
private:
    std::string name;
    std::vector<int64_t> layers;
};

#endif
