#include "fbx_model.h"

#include "fbx_scene.h"

unsigned FbxModel::ChildCount() const
{
    return children.size();
}

FbxModel& FbxModel::GetChild(unsigned i, FbxScene& scene)
{
    return scene.GetModelByUid(children[i]);
}

void FbxModel::_addChild(int64_t uid){
    children.emplace_back(uid);
}