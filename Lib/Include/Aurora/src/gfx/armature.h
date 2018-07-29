#ifndef SKELETON_H
#define SKELETON_H

#include <string>
#include <vector>

#include <stdint.h>

#include "../math/math.h"

namespace Au{
namespace GFX{

class Bone
{
public:
    Bone* CreateChild();
    Bone* GetChild(int id);
    Bone* GetChild(const std::string& name);
    Bone* Parent();
    
    Bone* BindTransform(const Math::Mat4f mat);
    
    int64_t uid;
    int64_t parentUID;
    Math::Mat4f bindTransform;
    std::string name;
};

class Armature
{
public:
    Bone* GetBone(int64_t uid)
    {
        for(unsigned i = 0; i < bones.size(); ++i)
            if(bones[i].uid == uid)
                return &(bones[i]);
            
        Bone bone;
        bone.uid = uid;
        bones.push_back(bone);
        return &(bones[bones.size() - 1]);
    }
    Bone* GetBones() { return bones.data(); }
    unsigned BoneCount() { return bones.size(); }
private:
    std::vector<Bone> bones;
    std::vector<std::string> boneNames;
    std::vector<Math::Mat4f> transforms;
};

}
}

#endif
