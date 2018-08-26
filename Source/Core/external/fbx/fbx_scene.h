#ifndef FBX_SCENE_H
#define FBX_SCENE_H

#include "fbx_node.h"
#include "fbx_model.h"
#include "fbx_mesh.h"
#include "fbx_light.h"
#include "fbx_geometry.h"
#include "fbx_connection.h"
#include "fbx_math.h"

#include <string>
#include <vector>
#include <iostream>

struct FbxSettings
{
    double scaleFactor = 1.0;
};

class FbxScene
{
public:
    unsigned ModelCount() const;
    FbxModel& GetModel(unsigned i);
    FbxModel& GetModelByUid(int64_t uid);
    FbxMesh& GetMesh(int64_t uid);
    unsigned GeometryCount() const;
    FbxGeometry& GetGeometry(unsigned i);
    FbxGeometry& GetGeometryByUid(int64_t uid);

    void _dumpFile(const std::string& filename);
    /* Don't use this */
    void _finalize();
    /* This is not a spatial node, it's a file node */
    FbxNode& _getRootNode() { return rootNode; }
private:
    void _makeGlobalSettings();
    FbxModel& _makeModel(FbxNode& node);
    void _makeMesh(FbxNode& node);
    FbxConnection* _findObjectToObjectParentConnection(int64_t uid);

    FbxNode rootNode;

    FbxSettings settings;

    std::vector<int64_t> rootModels;
    std::map<int64_t, FbxModel> models;

    std::map<int64_t, FbxMesh> meshes;
    std::vector<int64_t> geometryUids;
    std::map<int64_t, FbxGeometry> geometries;

    std::vector<FbxConnection> connections;
};

#endif
