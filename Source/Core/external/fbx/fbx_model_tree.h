#ifndef FBX_MODEL_TREE_H
#define FBX_MODEL_TREE_H

#include "fbx_scene.h"
#include "fbx_node.h"

class FbxConnection
{
public:
    FbxConnection() {}
    FbxConnection(FbxNode& node)
    {
        type = node.GetProperty(0).GetString();
        child_uid = node.GetProperty(1).GetInt64();
        parent_uid = node.GetProperty(2).GetInt64();
        
        if(node.PropCount() > 3)
        {
            name = node.GetProperty(3).GetString();
        }
    }

    std::string type;
    int64_t child_uid;
    int64_t parent_uid;    
    std::string name;
};

class FbxModel
{
public:
    FbxModel(){}
    FbxModel(FbxNode& node)
    {
        auto uid = node.GetProperty(0).GetInt64();
        auto name = node.GetProperty(1).GetString();
        auto type = node.GetProperty(2).GetString();

        std::cout << uid << ": " << name << ", " << type << std::endl;

        for(unsigned i = 0; i < root.ChildCount("C"); ++i)
        {
            FbxNode& fbxConn = root.GetNode("C", i);
            FbxConnection conn(fbxConn);
            if(conn.child_uid == uid)
            {
                std::cout << "Connected to " <<
                    conn.parent_uid << ", type - " <<
                    conn.type << std::endl;
            }
        }
    }
};

class FbxGeometry {};
class FbxPose {};
class FbxDeformer {};
class FbxMaterial {};
class FbxTexture {};
class FbxAnimationStack {};
class FbxAnimationCurveNode {};
class FbxAnimationCurve {};
class FbxSettings {}; // Properties70

class FbxMesh {};
class FbxLimbNode {};
class FbxLight {};
class FbxCamera {};

template<typename T>
class FbxNodeContainer
{
public:
    T& GetByUid(int64_t uid);
    void Add(T& node);
    void FindAll(FbxNode& rootNode, const std::string& fbxName)
    {
        for(unsigned i = 0; i < rootNode.ChildCount(fbxName); ++i){
            elements.emplace_back(T(rootNode.GetNode(fbxName, i)));
        }
    }

    std::vector<T> elements;
};

class FbxModelTree
{
public:
    FbxModelTree(FbxNode& rootNode)
    {
        for(unsigned i = 0; i < rootNode.ChildCount("C"); ++i){
            connections.emplace_back(FbxConnection(rootNode.GetNode("C", i)));
        }

        geometries.FindAll(rootNode, "Geometry");
        
        for(unsigned i = 0; i < rootNode.ChildCount("Model"); ++i){
            models.emplace_back(FbxModel(rootNode.GetNode("Model", i)));
        }
        for(unsigned i = 0; i < rootNode.ChildCount("Pose"); ++i){
            poses.emplace_back(FbxPose());
        }
        for(unsigned i = 0; i < rootNode.ChildCount("Deformer"); ++i){
            deformers.emplace_back(FbxDeformer());
        }
    }
private:
    FbxNodeContainer<FbxGeometry> geometries;

    std::vector<FbxModel> models;
    std::vector<FbxPose> poses;
    std::vector<FbxDeformer> deformers;
    std::vector<FbxMaterial> materials;
    std::vector<FbxTexture> textures;
    std::vector<FbxAnimationStack> animationStacks;
    std::vector<FbxConnection> connections;
};

#endif
