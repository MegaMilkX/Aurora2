#ifndef FBX_SKIN_H
#define FBX_SKIN_H

#include <vector>
#include <iostream>

#include "fbx_math.h"
#include "fbx_node.h"
#include "fbx_object.h"

class FbxDeformer : public FbxObject {
public:
    virtual const char* Type() const { return "Deformer"; }
    virtual bool Make(FbxNode& node);

    FbxMatrix4 transform;
    FbxMatrix4 transformAssociateModel;
    FbxMatrix4 transformLink;
    std::vector<int32_t> indices;
    std::vector<double> weights;
    std::string name;
    std::string boneName;
    int64_t targetModel;
};

class FbxSkin : public FbxObject {
public:
    virtual const char* Type() const { return "Deformer"; }
    virtual bool Make(FbxNode& node);

    const std::string& Name() const { return name; }
    size_t DeformerCount() const { return deformers.size(); }
    FbxDeformer* GetDeformer(size_t i);

    struct boneData {
        size_t bone;
        float weight;
    };
    std::vector<std::vector<boneData>> boneDataPerControlPoint;
    void Print() {
        std::cout << "Skin Deformer count: " << deformers.size() << std::endl;
    }
private:
    std::vector<int64_t> deformers;
    std::string name;
};

#endif
