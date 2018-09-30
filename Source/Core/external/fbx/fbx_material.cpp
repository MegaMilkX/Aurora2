#include "fbx_material.h"
#include "fbx_scene.h"

bool FbxMaterial::Make(FbxNode& node) {
    if(node.GetName() != Type()) {
        return false;
    }
    if(!scene) return false;
    uid = node.GetProperty(0).GetInt64();
    name = node.GetProperty(1).GetString();
    size_t prop_count = scene->Connections().CountChildren(FBX_CONNECTION_TYPE::FBX_OBJECT_PROPERTY, uid);
    for(size_t i = 0; i < prop_count; ++i) {
        FbxConnection* conn = scene->Connections().GetChildConnection(FBX_OBJECT_PROPERTY, uid, i);
        if(conn->name == "DiffuseColor") {
            diffuseColorUid = conn->child_uid;
        }
    }
    return true;
}

FbxTexture* FbxMaterial::GetDiffuseTexture() {
    if(!scene) return 0;
    return scene->GetByUid<FbxTexture>(diffuseColorUid);
}