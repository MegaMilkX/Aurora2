#ifndef FBX_MATERIAL_H
#define FBX_MATERIAL_H

#include "fbx_object.h"
#include "fbx_node.h"
#include "fbx_texture.h"

class FbxMaterial : public FbxObject {
public:
    virtual const char* Type() const { return "Material"; }
    virtual bool Make(FbxNode& node);

    const std::string& Name() const { return name; }
    FbxTexture* GetDiffuseTexture();
    int64_t GetDiffuseTextureUid() { return diffuseColorUid; }
private:
    std::string name;
    int64_t diffuseColorUid = 0;
};

#endif
