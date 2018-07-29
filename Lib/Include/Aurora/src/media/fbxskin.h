#ifndef AU_MEDIA_FBX_SKIN_H
#define AU_MEDIA_FBX_SKIN_H

#include "fbxnode.h"

namespace Au{
namespace Media{
namespace FBX{
    
class Skin
{
public:
    Skin() {}
    Skin(Node& rootNode, Node& geom)
    : uid(0)
    {
        int64_t uidGeom = geom[0].GetInt64();
        
        std::vector<Node*> deformers =
            rootNode.GetConnectedChildren("Deformer", uidGeom);
        if(deformers.empty())
            return;
        
        Node& deformer = *deformers[0];
        if(deformer[2].GetString() != "Skin")
            return;
        uid = deformer[0].GetInt64();
        
        deformers = 
            rootNode.GetConnectedChildren("Deformer", uid);
            
        for(unsigned i = 0; i < deformers.size(); ++i)
            bones.push_back((*deformers[i])[0].GetInt64());
    }
    
    unsigned BoneCount() { return bones.size(); }
    int64_t GetBoneUID(unsigned i)
    {
        int64_t uidBone = bones[i];
        return uidBone;
    }
    
    int64_t uid;
private:
    std::vector<int64_t> bones;
};
    
}
}
}

#endif
