#ifndef FBX_TEXTURE_H
#define FBX_TEXTURE_H

#include "fbx_object.h"
#include "fbx_node.h"

class FbxTexture : public FbxObject {
public:
    virtual const char* Type() const { return "Texture"; }
    virtual bool Make(FbxNode& node);

    const std::string& TextureName() const { return name; }
    const std::string& FileName() const { return filename; }
private:
    std::string name;
    std::string filename;
};

#endif
