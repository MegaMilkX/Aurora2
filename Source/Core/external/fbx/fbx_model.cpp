#include "fbx_model.h"
#include "fbx_pose.h"
#include "fbx_scene.h"

bool FbxModel::Make(FbxNode& node) {
    size_t oo_parent_count = scene->Connections().CountParents(FBX_OBJECT_OBJECT, GetUid());
    for(size_t i = 0; i < oo_parent_count; ++i) {
        parent_uid = scene->Connections().GetParent(FBX_OBJECT_OBJECT, GetUid(), i);
        FbxModel* mdl = scene->GetByUid<FbxModel>(parent_uid);
        if(mdl) {
            break;
        } else {
            parent_uid = 0;
        }
    }

    worldBindPose = FbxMatrix4(1.0f);
    lclBindPose = FbxMatrix4(1.0f);
    FbxPose* bindPose = scene->GetBindPose();
    if(bindPose) {
        auto it = bindPose->transforms.find(GetUid());
        if(it != bindPose->transforms.end()) {
            worldBindPose = it->second;
            FbxModel* parent = scene->GetByUid<FbxModel>(parent_uid);
            if(parent) {
                lclBindPose = parent->GetWorldBindPose() * FbxInverse(worldBindPose);
            } else {
                lclBindPose = worldBindPose;
            }
        }
    }
    return true;
}

unsigned FbxModel::ChildCount() const
{
    return children.size();
}

FbxModel* FbxModel::GetChild(unsigned i, FbxScene& scene)
{
    return scene.GetByUid<FbxModel>(children[i]);
}

void FbxModel::_addChild(int64_t uid){
    children.emplace_back(uid);
}