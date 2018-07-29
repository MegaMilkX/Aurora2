#ifndef AU_FBXBONE_H
#define AU_FBXBONE_H

#include "../math/math.h"

#include "fbxnode.h"

#include "fbxutil.h"

#include "fbxpose.h"

#include "fbxmesh.h"

namespace Au{
namespace Media{
namespace FBX{

class Bone
{
public:
    Bone(Settings& settings, Node& root, Node& node, Mesh* meshTgt)
    : transform(1.0f), uid(0), puid(0), deformerUID(0)
    {
        uid = node[0].GetInt64();
        name = node[1].GetString();
        
        Node* conn = 0;
        Node* parent = 
            root.GetConnectedParent("Model", uid, &conn);
            
        Node* poseData = root.GetWhere("Pose", 2, "BindPose");
        Pose pose(settings, &root, poseData);
        
        if(parent)
            puid = (*parent)[0].GetInt64();
        
        Au::Math::Vec3f pos(0.0f, 0.0f, 0.0f);
        Au::Math::Quat qrot(0.0f, 0.0f, 0.0f, 1.0f);
        Au::Math::Vec3f scale(1.0f, 1.0f, 1.0f);
        
        Node* deformer = 
            root.GetConnectedParent("Deformer", uid, &conn);
        if(!deformer)
            return;
        deformerUID = (*deformer)[0].GetInt64();
        
        if(!pose.GetPoseTransform(uid, transform))
        {/*
            std::vector<double> transformLinkD =
                deformer->Get("TransformLink")[0].GetArray<double>();
            
            if(transformLinkD.size() < 16)
                return;
            std::vector<float> transformLinkF;
            for(unsigned i = 0; i < transformLinkD.size(); ++i)
                transformLinkF.push_back((float)transformLinkD[i]);
            Au::Math::Mat4f mat4f = *(Math::Mat4f*)transformLinkF.data();
            
            transform = mat4f;*/
            
            SceneNode sn(settings, node);

            pos = sn.LclTranslation();            
            qrot = Au::Math::EulerToQuat(sn.LclRotation());
            scale = sn.LclScaling();
            
            transform = 
                Au::Math::Translate(Au::Math::Mat4f(1.0f), pos) * 
                Au::Math::ToMat4(qrot) * 
                Au::Math::Scale(Au::Math::Mat4f(1.0f), scale);
            
        }
        
        if(!meshTgt)
            return;
        
        meshName = meshTgt->name;
        
        
        indices = deformer->Get("Indexes")[0].GetArray<int32_t>();
        std::vector<double> w = 
            deformer->Get("Weights")[0].GetArray<double>();
        for(unsigned i = 0; i < w.size(); ++i)
            weights.push_back((float)w[i]);
        
        if(indices.size() != weights.size())
        {
            std::cout 
            << "Warning! " 
            << name 
            << " fbx bone has unequal index and weight counts" 
            << std::endl;
            indices.clear();
            weights.clear();
            return;
        }
        
        std::vector<int32_t> newIndices;
        std::vector<float> newWeights;
        for(unsigned i = 0; i < indices.size(); ++i)
        {
            for(unsigned j = 0; j < meshTgt->origVertIndices.size(); ++j)
            {
                if(meshTgt->origVertIndices[j] == indices[i])
                {
                    newIndices.push_back(meshTgt->origVertIndices[j]);
                    newWeights.push_back(weights[i]);
                }
            }
        }
        
        //indices = newIndices;
        //weights = newWeights;
    }
    
    void Index(unsigned i) { index = i; }
    unsigned Index() { return index; }
    
    bool IsDeformer() { return deformerUID != 0 ? true : false; }
    
    bool operator<(const Bone& other)
    {
        return name < other.name;
    }
    
    int64_t uid;
    int64_t deformerUID;
    int64_t puid;
    std::string name;
    std::string meshName;
    Au::Math::Mat4f transform;
    std::vector<int32_t> indices;
    std::vector<float> weights;
    
    unsigned index;
}; 

}
}
}

#endif
