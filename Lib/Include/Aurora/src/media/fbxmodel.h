#ifndef AU_FBXMODEL_H
#define AU_FBXMODEL_H

#include "../math/math.h"

#include "fbxnode.h"
#include "fbxscenenode.h"
#include "fbxpose.h"
#include "fbxutil.h"

namespace Au{
namespace Media{
namespace FBX{

class Model
{
public:
    Model() {}
    Model(Settings* settings, Node* root, Node* node)
    : settings(settings), 
      root(root), 
      node(node), 
      uid(0),
      parentUID(0), 
      deformerUID(0),
      transform(1.0f)
    {
        Node& n = *node;
        uid = n[0].GetInt64();
        name = n[1].GetString();
        type = n[2].GetString();
        
        Node* conn = 0;
        Node* parent = 
            root->GetConnectedParent("Model", uid, &conn);
        if(parent)
            parentUID = (*parent)[0].GetInt64();

        children = root->GetConnectedChildren("Model", uid);
        
        Node* poseData = root->GetWhere("Pose", 2, "BindPose");
        Pose pose(*settings, root, poseData);
        Node* deformer = 
            root->GetConnectedParent("Deformer", uid, &conn);
        if(deformer)
        {
            deformerUID = (*deformer)[0].GetInt64();
        }
        
        Au::Math::Vec3f pos(0.0f, 0.0f, 0.0f);
        Au::Math::Quat qrot(0.0f, 0.0f, 0.0f, 1.0f);
        Au::Math::Vec3f scale(1.0f, 1.0f, 1.0f);
        
        if(!pose.GetPoseTransform(uid, transform))
        {            
            SceneNode sn(*settings, *node);

            pos = sn.LclTranslation();
            Au::Math::Quat pre = sn.PreRotationQuat();
            Au::Math::Quat post = sn.PostRotationQuat();
            qrot = Au::Math::EulerToQuat(sn.LclRotation());
            scale = sn.LclScaling();
            
            transform = 
                Au::Math::Translate(Au::Math::Mat4f(1.0f), pos) * 
                Au::Math::ToMat4(pre * qrot * post) *
                Au::Math::Scale(Au::Math::Mat4f(1.0f), scale);
        }
    }
    
    bool IsBone() { return deformerUID != 0; }
    bool IsRoot() { return parentUID == 0; };
    unsigned ChildCount() { return children.size(); }
    Node* GetChild(unsigned i) { return children[i]; }
    
    Settings* settings;
    
    int64_t uid;
    int64_t parentUID;
    int64_t deformerUID;
    std::string name;
    std::string type;
    
    Au::Math::Mat4f transform;
    
    Node* root;
    Node* node;

    std::vector<Node*> children;
};    
 
}
}
}

#endif