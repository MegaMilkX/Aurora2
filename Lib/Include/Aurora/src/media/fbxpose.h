#ifndef AU_FBXPOSE_H
#define AU_FBXPOSE_H

#include <map>
#include <vector>

#include "fbxnode.h"
#include "fbxutil.h"
#include "fbxsettings.h"
#include "fbxscenenode.h"

namespace Au{
namespace Media{
namespace FBX{

struct PoseNode
{
    PoseNode(): uid(0), parentUid(0) {}
    int64_t uid;
    int64_t parentUid;
    Au::Math::Mat4f transform;
    Au::Math::Mat4f invWorldTransform;
    SceneNode sceneNode;
};

class Pose
{
public:
    Pose(Settings& settings, Node* root, Node* poseData)
    {
        if(!poseData)
            return;
        
        _settings = &settings;
        
        std::vector<Node> nodes =
            poseData->GetAll("PoseNode");
        for(unsigned i = 0; i < nodes.size(); ++i)
        {
            int64_t nodeUid = 
                nodes[i].Get("Node")[0].GetInt64();
            std::vector<double> md =
                nodes[i].Get("Matrix")[0].GetArray<double>();
            if(md.size() < 16)
                return;
            std::vector<float> mf;
            for(unsigned i = 0; i < md.size(); ++i)
                mf.push_back((float)md[i]);
            Au::Math::Mat4f mat4f = *(Math::Mat4f*)mf.data();
            mat4f[3] = 
                Au::Math::Vec4f(
                    mat4f[3].x * (float)settings.scaleFactor,
                    mat4f[3].y * (float)settings.scaleFactor,
                    mat4f[3].z * (float)settings.scaleFactor,
                    1.0f
                );
            
            PoseNode& p = poses[nodeUid];
            p.uid = nodeUid;
            p.invWorldTransform = Au::Math::Inverse(mat4f);
            
            Node* conn = 0;
            Node* parent = root->GetConnectedParent("Model", nodeUid, &conn);
            if(parent)
                p.parentUid = (*parent)[0].GetInt64();
            
            Node* thisNode = root->GetWhere("Model", 0, nodeUid);
            if(thisNode)
                p.sceneNode = SceneNode(settings, *thisNode);
        }
    }
    
    bool GetPoseTransform(int64_t uid, Au::Math::Mat4f& transform)
    {
        std::map<int64_t, PoseNode>::iterator it =
            poses.find(uid);
        if(it == poses.end())
            return false;
        
        PoseNode& pn = it->second;
        
        transform = pn.invWorldTransform;
        //transform = Au::Math::Inverse(transform);
        
        PoseNode* parent = _getParent(uid);
        if(parent)
            transform = parent->invWorldTransform * Au::Math::Inverse(transform);
        else
            transform = Au::Math::Inverse(transform);
        /*
        transform = 
            transform * 
            Au::Math::ToMat4(
                Au::Math::Inverse(
                    pn.sceneNode.PostRotationQuat()
                )
            );
        
        transform = 
            transform * 
            Au::Math::ToMat4(
                Au::Math::Inverse(
                    pn.sceneNode.PreRotationQuat()
                )
            );
        */
        return true;
    }
private:
    PoseNode* _getPose(int64_t uid)
    {
        std::map<int64_t, PoseNode>::iterator it =
            poses.find(uid);
        if(it == poses.end())
            return 0;
        return &it->second;
    }

    PoseNode* _getParent(int64_t uid)
    {
        PoseNode* pose = _getPose(uid);
        if(!pose)
            return 0;
        PoseNode* parent = _getPose(pose->parentUid);
        return parent;
    }
    std::map<int64_t, PoseNode> poses;
    Settings* _settings;
};

}
}
}

#endif
