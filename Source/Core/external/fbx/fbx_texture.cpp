#include "fbx_texture.h"

bool FbxTexture::Make(FbxNode& node) {
    if(!scene) return false;
    if(node.GetName() != Type()) {
        return false;
    }
    name = node.GetProperty(1).GetString();
    if(node.ChildCount("FileName")) {
        filename = node.GetNode("FileName", 0).GetProperty(0).GetString();
    }
    return true;
}