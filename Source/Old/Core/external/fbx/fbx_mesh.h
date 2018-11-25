#ifndef FBX_MESH_H
#define FBX_MESH_H

#include <stdint.h>
#include <string>
#include "fbx_object.h"
#include "fbx_geometry.h"
#include "fbx_model.h"

class FbxMesh : public FbxModel
{
public:
    void SetGeometryUid(int64_t uid) { geomUid = uid; }
    int64_t GetGeometryUid() const { return geomUid; }
private:
    int64_t geomUid;
};

#endif
