#ifndef SCENE_FROM_FBX_H
#define SCENE_FROM_FBX_H

#define MINIZ_HEADER_FILE_ONLY
#include "../../lib/miniz.c"

#include <animation_driver.h>
#include <skin.h>
#include <bone.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

typedef std::map<int64_t, std::shared_ptr<Mesh>> mesh_res_map_t;
typedef std::map<int64_t, std::shared_ptr<Animation>> anim_res_map_t;
typedef std::map<int64_t, std::shared_ptr<Skeleton>> skel_res_map_t;

struct FbxImportData {
    mesh_res_map_t meshes;
    anim_res_map_t anims;
    skel_res_map_t skeletons;

    void AddObject(int64_t fbxModelUid, SceneObject* so) {
        loadedObjects.emplace_back(so);
        fbxUidToObject[fbxModelUid] = so;
    }

    SceneObject* FindObject(const std::string& name) {
        for(auto so : loadedObjects) {
            if(so->Name() == name) return so;
        }
        return 0;
    }
    SceneObject* FindObjectByUid(int64_t uid) {
        auto it = fbxUidToObject.find(uid);
        if(it == fbxUidToObject.end()) return 0;
        return it->second;
    }

    void Root(SceneObject* so) { root = so; }
    SceneObject* Root() { return root; }
private:
    SceneObject* root;
    std::vector<SceneObject*> loadedObjects;
    std::map<int64_t, SceneObject*> fbxUidToObject;
};

inline void ResourcesFromFbxScene(const aiScene* ai_scene, FbxImportData& importData) {
    unsigned int mesh_count = ai_scene->mNumMeshes;
    for(unsigned int i = 0; i < mesh_count; ++i) {
        aiMesh* ai_mesh = ai_scene->mMeshes[i];
        int32_t vertexCount = ai_mesh->mNumVertices;
        int32_t triCount = ai_mesh->mNumFaces;
        int32_t indexCount = triCount * 3;

        std::vector<float> vertices;
        std::vector<uint32_t> indices;
        std::vector<std::vector<float>> normal_layers;
        normal_layers.resize(1);
        std::vector<std::vector<float>> uv_layers;
        std::vector<std::vector<float>> rgb_layers;
        std::vector<gfxm::vec4> boneIndices;
        std::vector<gfxm::vec4> boneWeights;

        for(unsigned j = 0; j < vertexCount; ++j) {
            aiVector3D& v = ai_mesh->mVertices[j];
            vertices.emplace_back(v.x);
            vertices.emplace_back(v.y);
            vertices.emplace_back(v.z);
        }
        for(unsigned j = 0; j < triCount; ++j) {
            aiFace& f = ai_mesh->mFaces[j];
            for(unsigned k = 0; k < f.mNumIndices; ++k) {
                indices.emplace_back(f.mIndices[k]);
            }
        }
        for(unsigned j = 0; j < vertexCount; ++j) {
            aiVector3D& n = ai_mesh->mNormals[j];
            normal_layers[0].emplace_back(n.x);
            normal_layers[0].emplace_back(n.y);
            normal_layers[0].emplace_back(n.z);
        }
        for(unsigned j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++j) {
            if(!ai_mesh->HasTextureCoords(j)) break;
            std::vector<float> uv_layer;
            for(unsigned k = 0; k < vertexCount; ++k) {
                aiVector3D& uv = ai_mesh->mTextureCoords[j][k];
                uv_layer.emplace_back(uv.x);
                uv_layer.emplace_back(uv.y);
            }
            uv_layers.emplace_back(uv_layer);
        }

        if(ai_mesh->mNumBones) {
            boneIndices.resize(vertexCount);
            boneWeights.resize(vertexCount);
            for(unsigned j = 0; j < ai_mesh->mNumBones; ++j) {
                unsigned int bone_index = j;
                aiBone* bone = ai_mesh->mBones[j];
                for(unsigned k = 0; k < bone->mNumWeights; ++k) {
                    aiVertexWeight& w = bone->mWeights[k];
                    gfxm::vec4& indices_ref = boneIndices[w.mVertexId];
                    gfxm::vec4& weights_ref = boneWeights[w.mVertexId];
                    for(unsigned l = 0; l < 4; ++l) {
                        if(weights_ref[l] == 0.0f) {
                            indices_ref[l] = (float)bone_index;
                            weights_ref[l] = w.mWeight;
                            break;
                        }
                    }
                }
            }
        }

        std::shared_ptr<Mesh> mesh_ref(new Mesh());
        mesh_ref->Name(MKSTR(i << ".geo"));
        mesh_ref->Storage(Resource::LOCAL);
        /*
        for(size_t j = 0; j < indices.size(); j+=3) {
            LOG("TRI: " << indices[j] << ", " << indices[j+1] << ", " << indices[j+2]);
        }*/

        mesh_ref->SetAttribArray<Au::Position>(vertices);
        if(normal_layers.size() > 0) {
            mesh_ref->SetAttribArray<Au::Normal>(normal_layers[0]);
        }
        if(uv_layers.size() > 0) {
            mesh_ref->SetAttribArray<Au::UV>(uv_layers[0]);
        }
        if(!boneIndices.empty() && !boneWeights.empty()) {
            mesh_ref->SetAttribArray<Au::BoneIndex4>(boneIndices);
            mesh_ref->SetAttribArray<Au::BoneWeight4>(boneWeights);
            /*
            for(unsigned j = 0; j < boneWeights.size(); ++j) {
                gfxm::vec4& v = boneWeights[j];
                gfxm::vec4& bi = boneIndices[j];
                LOG(
                    "[" << bi.x << ", " << bi.y << ", " << bi.z << ", " << bi.w << "]: " <<
                    v.x << ", " << v.y << ", " << v.z << ", " << v.w
                );
                if(v.x == 0.0f && v.y == 0.0f && v.z == 0.0f && v.w == 0.0f) {
                    LOG("EMPTY BONE WEIGHTS FOR VERTEX " << j)
                }
            }*/
        }
        mesh_ref->SetIndices(indices);

        importData.meshes[i] = mesh_ref;

        // === Skin data
        if(ai_mesh->mNumBones) {
            std::shared_ptr<Skeleton> skeleton(new Skeleton());
            skeleton->Name(MKSTR(i << ".skel"));
            skeleton->Storage(Resource::LOCAL);

            for(unsigned j = 0; j < ai_mesh->mNumBones; ++j) {
                aiBone* bone = ai_mesh->mBones[j];
                std::string name(bone->mName.data, bone->mName.length);

                skeleton->AddBone(name, gfxm::transpose(*(gfxm::mat4*)&bone->mOffsetMatrix));
            }

            importData.skeletons[i] = skeleton;
        }
    }

    for(unsigned int i = 0; i < ai_scene->mNumAnimations; ++i) {
        aiAnimation* ai_anim = ai_scene->mAnimations[i];
        double fps = ai_anim->mTicksPerSecond;
        double len = ai_anim->mDuration;
        std::shared_ptr<Animation> anim = std::make_shared<Animation>();
        anim->FrameRate(fps);
        anim->Length(len);

        for(unsigned int j = 0; j < ai_anim->mNumChannels; ++j) {
            AnimationNode animNode;
            aiNodeAnim* ai_node_anim = ai_anim->mChannels[j];
            animNode.name = ai_node_anim->mNodeName.data;
            
            std::vector<keyframe<gfxm::vec3>> pos_frames;
            std::vector<keyframe<gfxm::quat>> rot_frames;
            std::vector<keyframe<gfxm::vec3>> scl_frames;
            
            for(unsigned k = 0; k < ai_node_anim->mNumPositionKeys; ++k) {
                keyframe<gfxm::vec3> kf;
                kf.time = (float)ai_node_anim->mPositionKeys[k].mTime;
                kf.value = gfxm::vec3(
                    ai_node_anim->mPositionKeys[k].mValue.x,
                    ai_node_anim->mPositionKeys[k].mValue.y,
                    ai_node_anim->mPositionKeys[k].mValue.z
                );
                pos_frames.emplace_back(kf);
            }
            for(unsigned k = 0; k < ai_node_anim->mNumRotationKeys; ++k) {
                keyframe<gfxm::quat> kf;
                kf.time = (float)ai_node_anim->mRotationKeys[k].mTime;
                kf.value = gfxm::quat(
                    ai_node_anim->mRotationKeys[k].mValue.x,
                    ai_node_anim->mRotationKeys[k].mValue.y,
                    ai_node_anim->mRotationKeys[k].mValue.z,
                    ai_node_anim->mRotationKeys[k].mValue.w
                );
                rot_frames.emplace_back(kf);
            }
            for(unsigned k = 0; k < ai_node_anim->mNumScalingKeys; ++k) {
                keyframe<gfxm::vec3> kf;
                kf.time = (float)ai_node_anim->mScalingKeys[k].mTime;
                kf.value = gfxm::vec3(
                    ai_node_anim->mScalingKeys[k].mValue.x,
                    ai_node_anim->mScalingKeys[k].mValue.y,
                    ai_node_anim->mScalingKeys[k].mValue.z
                );
                scl_frames.emplace_back(kf);
            }

            if(!pos_frames.empty()) {
                animNode.children["Transform"].name = "Transform";
                animNode.children["Transform"].children["Translation"].name = "Translation";
                animNode.children["Transform"].children["Translation"].InitCurve<gfxm::vec3>();
                animNode.children["Transform"].children["Translation"].GetCurve<gfxm::vec3>()->set_keyframes(pos_frames);
            }
            if(!rot_frames.empty()) {
                AnimationNode& comp_node =
                    animNode.children["Transform"];
                comp_node.name = "Transform";
                AnimationNode& prop_node =
                    comp_node.children["Quaternion"];
                prop_node.name = "Quaternion";
                prop_node.InitCurve<gfxm::quat>();
                prop_node.GetCurve<gfxm::quat>()->set_keyframes(rot_frames);
            }
            if(!scl_frames.empty()) {
                AnimationNode& comp_node =
                    animNode.children["Transform"];
                comp_node.name = "Transform";
                AnimationNode& prop_node = 
                    comp_node.children["Scale"];
                prop_node.name = "Scale";
                prop_node.InitCurve<gfxm::vec3>();
                prop_node.GetCurve<gfxm::vec3>()->set_keyframes(scl_frames);
            }
            anim->AddNode(animNode);
        }

        importData.anims[i] = anim;
    }
}

inline void SceneObjectFromFbxNode(const aiScene* ai_scene, aiNode* node, SceneObject* object, FbxImportData& importData) {
    std::string name(node->mName.data, node->mName.length);
    object->Name(name);
    object->Get<Transform>()->SetTransform(gfxm::transpose(*(gfxm::mat4*)&node->mTransformation));
    for(unsigned int i = 0; i < node->mNumChildren; ++i) {
        SceneObject* child = object->CreateObject();
        object->Get<Transform>()->Attach(child->Get<Transform>());
        SceneObjectFromFbxNode(ai_scene, node->mChildren[i], child, importData);
    }

    if(node->mNumMeshes > 0) {
        auto it = importData.meshes.find(node->mMeshes[0]);
        if(it != importData.meshes.end()) {
            object->Get<Model>()->mesh = it->second;
        }
        aiMesh* ai_mesh = ai_scene->mMeshes[node->mMeshes[0]];
        if(ai_mesh->mNumBones) {
            Skin* skin = object->Get<Skin>();
            skin->SetArmatureRoot(importData.Root()->WeakPtr());
            auto it = importData.skeletons.find(node->mMeshes[0]);
            if(it != importData.skeletons.end()) {
                resource_ref<Skeleton> skel_res_ref;
                skel_res_ref = it->second;
                skin->SetSkeleton(skel_res_ref);
            }
            skin->SetBindTransform(gfxm::mat4(1.0f));
        }
    }
}

inline void SceneFromFbx(const aiScene* ai_scene, SceneObject* scene, FbxImportData& importData) {
    aiNode* ai_rootNode = ai_scene->mRootNode;
    if(!ai_rootNode) return;
    scene->Get<Transform>()->SetTransform(gfxm::transpose(*(gfxm::mat4*)&ai_rootNode->mTransformation));
    
    if(ai_scene->mMetaData) {
        double scaleFactor = 1.0;
        ai_scene->mMetaData->Get("UnitScaleFactor", scaleFactor);
        if(scaleFactor == 0.0) scaleFactor = 1.0;
        scaleFactor *= 0.01;
        scene->Get<Transform>()->Scale((float)scaleFactor);
    }
    //double scaleFactor = 1.0;
    //ai_rootNode->mMetaData->Get("UnitScaleFactor", scaleFactor);
    //scaleFactor *= 0.01;
    //scene->Get<Transform>()->Scale(gfxm::vec3(1.0f, 1.0f, 1.0f) * 0.01f);
    
    for(unsigned int i = 0; i < ai_rootNode->mNumChildren; ++i) {
        SceneObject* child = scene->CreateObject();
        scene->Get<Transform>()->Attach(child->Get<Transform>());
        SceneObjectFromFbxNode(ai_scene, ai_rootNode->mChildren[i], child, importData);
    }

    // TODO: Create AnimationDriver
    for(unsigned int i = 0; i < ai_scene->mNumAnimations; ++i) {
        auto animDriver = scene->Get<AnimationDriver>();
        auto ai_anim = ai_scene->mAnimations[i];
        animDriver->AddAnim(ai_anim->mName.data, importData.anims[i]);
    }
}

inline bool SceneFromFbx(const std::string& filename, SceneObject* scene)
{
    Assimp::Importer importer;
    const aiScene* ai_scene = importer.ReadFile(
        filename, 
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate | 
        aiProcess_JoinIdenticalVertices |
        aiProcess_LimitBoneWeights |
        aiProcess_GlobalScale |
        aiProcess_GenUVCoords
        //| aiProcess_OptimizeGraph
    );
    if(!ai_scene) {
        LOG("Failed to read " << filename);
        return false;
    }

    FbxImportData importData;
    importData.Root(scene);

    ResourcesFromFbxScene(ai_scene, importData);
    SceneFromFbx(ai_scene, scene, importData);

    return true;
}

#endif
