#ifndef FBX_MODEL_H
#define FBX_MODEL_H

#include "fbx_object.h"
#include "fbx_node.h"
#include "fbx_math.h"
#include <vector>
#include <string>

class FbxScene;

class FbxModel : public FbxObject
{
public:
    FbxModel(){}

    virtual bool Make(FbxNode& node) {
        return true;
    }

    unsigned ChildCount() const;
    FbxModel* GetChild(unsigned i, FbxScene& scene);

    void SetName(const std::string& name) { this->name = name; }
    const std::string& GetName() const { return name; }
    void SetType(const std::string& type) { this->type = type; }
    const std::string& GetType() const { return type; }

    void SetLclTranslation(const FbxVector3& arg) { lclTranslation = arg; }
    void SetLclRotation(const FbxVector3& arg) { lclRotation = arg; }
    void SetLclScaling(const FbxVector3& arg) { lclScaling = arg; }
    void SetPreRotation(const FbxVector3& arg) { preRotation = arg; }
    void SetPostRotation(const FbxVector3& arg) { postRotation = arg; }
    FbxVector3 GetLclTranslation() { return lclTranslation; }
    FbxVector3 GetLclRotation() { return lclRotation; }
    FbxVector3 GetLclScaling() { return lclScaling; }
    FbxVector3 GetPreRotation() { return preRotation; }
    FbxVector3 GetPostRotation() { return postRotation; }

    void SetTransform(const FbxMatrix4& t) { lclTransform = t; }
    FbxMatrix4 GetTransform() { return lclTransform; }

    void _addChild(int64_t uid);
private:
    std::vector<int64_t> children;
    std::string name;
    std::string type;
    FbxVector3 preRotation;
    FbxVector3 postRotation;
    FbxVector3 lclTranslation;
    FbxVector3 lclRotation;
    FbxVector3 lclScaling;

    FbxMatrix4 lclTransform;
};

#endif
