#include "fbx_skin.h"
#include "fbx_scene.h"
#include <algorithm>

bool FbxDeformer::Make(FbxNode& node) {
    std::string sub_type = node.GetProperty(2).GetString();
    if(sub_type != "Cluster") {
        std::cout << "FBX: Expected node subtype 'Cluster', got '" << sub_type << "'" << std::endl;
        return false;
    }

    name = node.GetProperty(1).GetString();

    if(node.ChildCount("Indexes")) {
        FbxNode& nodeIndexes = node.GetNode("Indexes", 0);
        indices = nodeIndexes.GetProperty(0).GetArray<int32_t>();
    }
    if(node.ChildCount("Weights")) {
        FbxNode& nodeWeights = node.GetNode("Weights", 0);
        weights = nodeWeights.GetProperty(0).GetArray<double>();
    }
    if(node.ChildCount("Transform")) {
        FbxNode& nodeTransform = node.GetNode("Transform", 0);
        std::vector<double> t = nodeTransform.GetProperty(0).GetArray<double>();
        for(size_t i = 0; i < 16; ++i) {
            ((float*)&transform)[i] = (float)t[i];
        }
    }
    if(node.ChildCount("TransformAssociateModel")) {
        FbxNode& nodeTransformAssociateModel = node.GetNode("TransformAssociateModel", 0);
        std::vector<double> t = nodeTransformAssociateModel.GetProperty(0).GetArray<double>();
        for(size_t i = 0; i < 16; ++i) {
            ((float*)&transformAssociateModel)[i] = (float)t[i];
        }
    }
    if(node.ChildCount("TransformLink")) {
        FbxNode& nodeTransformLink = node.GetNode("TransformLink", 0);
        std::vector<double> t = nodeTransformLink.GetProperty(0).GetArray<double>();
        for(size_t i = 0; i < 16; ++i) {
            ((float*)&transformLink)[i] = (float)t[i];
        }
    }
    return true;
}

bool FbxSkin::Make(FbxNode& node) {
    name = node.GetProperty(1).GetString();
    std::string sub_type = node.GetProperty(2).GetString();
    if(sub_type != "Skin") {
        std::cout << "FBX: Expected node subtype 'Skin', got '" << sub_type << "'" << std::endl;
        return false;
    }
    
    size_t oo_child_count = scene->Connections().CountChildren(FBX_OBJECT_OBJECT, GetUid());
    for(size_t i = 0; i < oo_child_count; ++i) {
        int64_t child_uid = scene->Connections().GetChild(FBX_OBJECT_OBJECT, GetUid(), i);
        FbxDeformer* deformer = scene->GetByUid<FbxDeformer>(child_uid);
        if(!deformer) continue;

        deformers.emplace_back(child_uid);
    }

    std::vector<int> boneCountPerVertex;
    std::sort(
        deformers.begin(), 
        deformers.end(), 
        [this](const int64_t& a, const int64_t& b) ->bool {
            FbxDeformer* da = scene->GetByUid<FbxDeformer>(a);
            FbxDeformer* db = scene->GetByUid<FbxDeformer>(b);
            return da->name < db->name;
        }
    );
    for(size_t i = 0; i < deformers.size(); ++i) {
        FbxDeformer* deformer = scene->GetByUid<FbxDeformer>(deformers[i]);
        for(size_t j = 0; j < deformer->indices.size() && j < deformer->weights.size(); ++j) {
            int32_t index = deformer->indices[j];
            double weight = deformer->weights[j];
            if(index >= boneCountPerVertex.size()) {
                boneCountPerVertex.resize(index + 1);
                boneDataPerControlPoint.resize(index + 1);
            }
            int vertexBoneId = boneCountPerVertex[index];
            boneDataPerControlPoint[index].emplace_back(boneData{i, (float)weight});
            boneCountPerVertex[index]++;
        }
    }
    
    std::cout << boneCountPerVertex.size() << " skinned vertices" << std::endl;

    return true;
}

FbxDeformer* FbxSkin::GetDeformer(size_t i) {
    return scene->GetByUid<FbxDeformer>(deformers[i]);
}