#ifndef FBX_DEFORMER_CLUSTER_2_H
#define FBX_DEFORMER_CLUSTER_2_H

#include "fbx_object.h"
#include "fbx_log.h"
#include "fbx_macro.h"

namespace Fbx {

class DeformerCluster : public Object {
public:
    virtual bool Make(Node& node) {
        Node* n = 0;
        if(n = node.FindNode("Indexes")) {
            if(!n->GetProperty(0).IsInt32Array()) {
                FBX_LOGW("Unexpected deformer " << GetUid() << " index format (Indexes)");
            } else {
                indices = n->GetProperty(0).GetArray<int32_t>();
            }
        } else {
            FBX_LOGW("Deformer " << GetUid() << " cluster has no indices");
        }

        if(n = node.FindNode("Weights")) {
            if(!n->GetProperty(0).IsDoubleArray()) {
                FBX_LOGW("Unexpected deformer " << GetUid() << " weights format (Weights)");
            } else {
                weights = n->GetProperty(0).GetArray<double>();
            }
        } else if(n = node.FindNode("BlendWeights")) {
            if(!n->GetProperty(0).IsDoubleArray()) {
                FBX_LOGW("Unexpected deformer " << GetUid() << " weights format (BlendWeights)");
            } else {
                weights = n->GetProperty(0).GetArray<double>();
            }
        } else {
            FBX_LOGW("Deformer " << GetUid() << " cluster has no weights");
        }
        
        if(n = node.FindNode("Transform")) {
            if(!n->GetProperty(0).IsDoubleArray()) {
                FBX_LOGW("Unexpected deformer " << GetUid() << " Transform format");
            } else {
                std::vector<double> data =
                    n->GetProperty(0).GetArray<double>();
                FbxDMatrix4 t = *(FbxDMatrix4*)data.data();
                transform = (t);
            }
        } else {
            //FBX_LOGW("Deformer has no Transform");
        }

        if(n = node.FindNode("TransformAssociateModel")) {
            if(!n->GetProperty(0).IsDoubleArray()) {
                FBX_LOGW("Unexpected deformer " << GetUid() << " TransformAssociateModel format");
            } else {
                std::vector<double> data =
                    n->GetProperty(0).GetArray<double>();
                FbxDMatrix4 t = *(FbxDMatrix4*)data.data();
                transformAssociateModel = (t);
            }
        } else {
            //FBX_LOGW("Deformer has no TransformAssociateModel");
        }

        if(n = node.FindNode("TransformLink")) {
            if(!n->GetProperty(0).IsDoubleArray()) {
                FBX_LOGW("Unexpected deformer " << GetUid() << " TransformLink format");
            } else {
                std::vector<double> data =
                    n->GetProperty(0).GetArray<double>();
                FbxDMatrix4 t = *(FbxDMatrix4*)data.data();
                transformLink = (t);
            }
        } else {
            FBX_LOGW("Deformer " << GetUid() << " has no TransformLink");
        }
        
        return true;
    }

    static bool IdentifyNode(Node& node) {
        if(node.GetName() != "Deformer"
            || node.PropCount() < 3
            || !node.GetProperty(0).IsInt64()
            || !node.GetProperty(1).IsString()
            || !node.GetProperty(2).IsString()
            || node.GetProperty(2).GetString() != "Cluster") 
            return false;
        return true;
    }

    std::vector<int32_t> indices;
    std::vector<double> weights;
    FbxMatrix4 transform;
    FbxMatrix4 transformLink;
    FbxMatrix4 transformAssociateModel;
};

}

#endif