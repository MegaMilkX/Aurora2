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
        std::vector<int32_t> indices;
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

        mz_zip_archive zip;
        memset(&zip, 0, sizeof(zip));
        mz_zip_writer_init_heap(&zip, 0, 0);

        mz_zip_writer_add_mem(&zip, "VertexCount", (void*)&vertexCount, sizeof(vertexCount), 0);
        mz_zip_writer_add_mem(&zip, "IndexCount", (void*)&indexCount, sizeof(indexCount), 0);
        mz_zip_writer_add_mem(&zip, "Indices", (void*)indices.data(), indexCount * sizeof(uint32_t), 0);
        mz_zip_writer_add_mem(&zip, "Vertices", (void*)vertices.data(), vertexCount * 3 * sizeof(float), 0);
        
        
        for(size_t j = 0; j < normal_layers.size(); ++j)
        {
            mz_zip_writer_add_mem(&zip, MKSTR("Normals." << j).c_str(), (void*)normal_layers[j].data(), vertexCount * 3 * sizeof(float), 0);
        }
        /*
        for(size_t j = 0; j < fbxMesh.UVLayerCount(); ++j)
        {
            mz_zip_writer_add_mem(&zip, MKSTR("UV." << j).c_str(), (void*)fbxMesh.GetUV(0).data(), vertexCount * 2 * sizeof(float), 0);
        }
*/
        if(ai_mesh->mNumBones) {
            boneIndices.resize(vertexCount);
            boneWeights.resize(vertexCount);
            std::fill(boneIndices.begin(), boneIndices.end(), gfxm::vec4(-1.0f, -1.0f, -1.0f, -1.0f));
            for(unsigned j = 0; j < ai_mesh->mNumBones; ++j) {
                unsigned int bone_index = j;
                aiBone* bone = ai_mesh->mBones[j];
                for(unsigned k = 0; k < bone->mNumWeights && k < 4; ++k) {
                    aiVertexWeight& w = bone->mWeights[k];
                    gfxm::vec4& indices_ref = boneIndices[w.mVertexId];
                    gfxm::vec4& weights_ref = boneWeights[w.mVertexId];
                    for(unsigned l = 0; l < 4; ++l) {
                        if(indices_ref[l] < 0.0f) {
                            indices_ref[l] = (float)bone_index;
                            weights_ref[l] = w.mWeight;
                            break;
                        }
                    }
                }
            }
            for(unsigned j = 0; j < boneIndices.size(); ++j) {
                for(unsigned k = 0; k < 4; ++k) {
                    if(boneIndices[j][k] < 0.0f) {
                        boneIndices[j][k] = 0.0f;
                    }
                }
            }

            mz_zip_writer_add_mem(&zip, "BoneIndices4", (void*)boneIndices.data(), boneIndices.size() * sizeof(gfxm::vec4), 0);
            mz_zip_writer_add_mem(&zip, "BoneWeights4", (void*)boneWeights.data(), boneWeights.size() * sizeof(gfxm::vec4), 0);
        }
/*
        LOG("Making a mesh resource: " << vertexCount << " vertices, " << indexCount << " indices");

        // TODO: Skin data load
        Fbx::DeformerSkin* fbxSkin = 
            fbxScene.GetChild<Fbx::DeformerSkin>(Fbx::OBJECT_OBJECT, geom->GetUid());
        if(fbxSkin) {
            std::vector<FbxVector4> boneIndices;
            std::vector<FbxVector4> boneWeights;
            if(MakeBlendIndicesAndWeights(fbxScene, fbxSkin, geom, fbxMesh, boneIndices, boneWeights)) {
                if(!boneIndices.empty() && !boneWeights.empty()) {
                    mz_zip_writer_add_mem(&zip, "BoneIndices4", (void*)boneIndices.data(), boneIndices.size() * sizeof(FbxVector4), 0);
                    mz_zip_writer_add_mem(&zip, "BoneWeights4", (void*)boneWeights.data(), boneWeights.size() * sizeof(FbxVector4), 0);
                }
            }
        }
*/
        void* bufptr;
        size_t sz;
        mz_zip_writer_finalize_heap_archive(&zip, &bufptr, &sz);

        DataSourceRef data_ref(new DataSourceMemory((char*)bufptr, sz));
        std::shared_ptr<Mesh> mesh_ref(new Mesh());
        mesh_ref->Build(data_ref);
        mesh_ref->Name(MKSTR(i << ".geo"));
        mesh_ref->Storage(Resource::LOCAL);
        importData.meshes[i] = mesh_ref;

        mz_zip_writer_end(&zip);
    }
}

inline void SceneObjectFromFbxNode(aiNode* node, SceneObject* object, FbxImportData& importData) {
    std::string name(node->mName.data, node->mName.length);
    object->Name(name);
    object->Get<Transform>()->SetTransform(gfxm::transpose(*(gfxm::mat4*)&node->mTransformation));
    for(unsigned int i = 0; i < node->mNumChildren; ++i) {
        SceneObject* child = object->CreateObject();
        object->Get<Transform>()->Attach(child->Get<Transform>());
        SceneObjectFromFbxNode(node->mChildren[i], child, importData);
    }

    if(node->mNumMeshes > 0) {
        auto it = importData.meshes.find(node->mMeshes[0]);
        if(it != importData.meshes.end()) {
            object->Get<Model>()->mesh = it->second;
        }
    }
}

inline void SceneFromFbx(const aiScene* ai_scene, SceneObject* scene, FbxImportData& importData) {
    aiNode* ai_rootNode = ai_scene->mRootNode;
    if(!ai_rootNode) return;
    //scene->Get<Transform>()->SetTransform(*(gfxm::mat4*)&ai_rootNode->mTransformation);
    
    //double scaleFactor = 1.0;
    //ai_rootNode->mMetaData->Get("UnitScaleFactor", scaleFactor);
    //scaleFactor *= 0.01;
    //scene->Get<Transform>()->Scale(gfxm::vec3(1.0f, 1.0f, 1.0f) * 0.01f);
    
    for(unsigned int i = 0; i < ai_rootNode->mNumChildren; ++i) {
        SceneObject* child = scene->CreateObject();
        scene->Get<Transform>()->Attach(child->Get<Transform>());
        SceneObjectFromFbxNode(ai_rootNode->mChildren[i], child, importData);
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
        aiProcess_GlobalScale
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
