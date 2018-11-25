#ifndef FBX_OBJECT_H
#define FBX_OBJECT_H

#include <stdint.h>
#include "fbx_node.h"

class FbxScene;
class FbxObject
{
public:
    virtual ~FbxObject() {}
    void SetUid(int64_t uid) { this->uid = uid; }
    int64_t GetUid() const { return uid; }

    void SetScene(FbxScene* scn) { scene = scn; }

    virtual const char* Type() const = 0;
    virtual bool Make(FbxNode& node) = 0;
protected:
    int64_t uid;
    FbxScene* scene;
};

#endif
