#include "fbx_pose.h"
#include "fbx_scene.h"

bool FbxPose::Make(FbxNode& node) {
    if(node.GetName() != Type()) {
        return false;
    }
    subtype = node.GetProperty(2).GetString();

    for(unsigned int i = 0; i < node.ChildCount("PoseNode"); ++i) {
        FbxNode& poseNode = node.GetNode("PoseNode", i);

        FbxNode& matrixNode = poseNode.GetNode("Matrix", 0);
        FbxNode& modelNode = poseNode.GetNode("Node", 0);

        std::vector<double> matrix = matrixNode.GetProperty(0).GetArray<double>();
        int64_t targetModelUid = modelNode.GetProperty(0).GetInt64();
        FbxMatrix4 mat;

        for(int j = 0; j < 16; ++j) {
            ((float*)&mat)[j] = (float)matrix[j];
        }

        mat[3] = 
            FbxVector4(
                mat[3].x * (float)scene->Settings().scaleFactor,
                mat[3].y * (float)scene->Settings().scaleFactor,
                mat[3].z * (float)scene->Settings().scaleFactor,
                1.0f
            );

        transforms.insert(std::make_pair(targetModelUid, mat));
    }
    
    return true;
}