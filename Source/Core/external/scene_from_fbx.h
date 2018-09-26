#ifndef SCENE_FROM_FBX_H
#define SCENE_FROM_FBX_H

#include "fbx/fbx_read.h"
#define MINIZ_HEADER_FILE_ONLY
#include "../../lib/miniz.c"
#include <component.h>

#include <animation_driver.h>
#include <skin.h>
#include <bone.h>
//#include <skeleton_animator.h>

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
private:
    std::vector<SceneObject*> loadedObjects;
    std::map<int64_t, SceneObject*> fbxUidToObject;
};

inline void ResourcesFromFbxScene(FbxScene& fbxScene, FbxImportData& importData)
{
    for(unsigned i = 0; i < fbxScene.Count<FbxGeometry>(); ++i)
    {
        FbxGeometry* geom = fbxScene.Get<FbxGeometry>(i);
        FbxIndexedTriangleMesh fbxMesh = geom->MakeIndexedMesh();
        
        mz_zip_archive zip;
        memset(&zip, 0, sizeof(zip));
        mz_zip_writer_init_heap(&zip, 0, 0);

        int32_t vertexCount = (int32_t)fbxMesh.VertexCount();
        int32_t indexCount = (int32_t)fbxMesh.IndexCount();
        mz_zip_writer_add_mem(&zip, "VertexCount", (void*)&vertexCount, sizeof(vertexCount), 0);
        mz_zip_writer_add_mem(&zip, "IndexCount", (void*)&indexCount, sizeof(indexCount), 0);
        mz_zip_writer_add_mem(&zip, "Indices", (void*)fbxMesh.GetIndices().data(), indexCount * sizeof(uint32_t), 0);
        mz_zip_writer_add_mem(&zip, "Vertices", (void*)fbxMesh.GetVertices().data(), vertexCount * 3 * sizeof(float), 0);
        for(size_t j = 0; j < fbxMesh.NormalLayerCount(); ++j)
        {
            mz_zip_writer_add_mem(&zip, MKSTR("Normals." << j).c_str(), (void*)fbxMesh.GetNormals(0).data(), vertexCount * 3 * sizeof(float), 0);
        }
        for(size_t j = 0; j < fbxMesh.UVLayerCount(); ++j)
        {
            mz_zip_writer_add_mem(&zip, MKSTR("UV." << j).c_str(), (void*)fbxMesh.GetUV(0).data(), vertexCount * 2 * sizeof(float), 0);
        }

        // TODO: Skin data load
        std::vector<int32_t> boneIndices4 = fbxMesh.GetBoneIndices4();
        std::vector<float> boneIndices4f;
        for(auto i : boneIndices4) {
            boneIndices4f.emplace_back((float)i);
        }
        std::vector<float> boneWeights4 = fbxMesh.GetBoneWeights4();
        if(!boneIndices4.empty() && !boneWeights4.empty()) {
            mz_zip_writer_add_mem(&zip, "BoneIndices4", (void*)boneIndices4f.data(), boneIndices4f.size() * sizeof(float), 0);
            mz_zip_writer_add_mem(&zip, "BoneWeights4", (void*)boneWeights4.data(), boneWeights4.size() * sizeof(float), 0);
        }

        FbxSkin* skin = geom->GetSkin();
        FbxPose* pose = fbxScene.GetBindPose();
        if(skin && pose) {
            std::shared_ptr<Skeleton> skeleton(new Skeleton());
            skeleton->Name(skin->Name());
            skeleton->Storage(Resource::LOCAL);
            for(size_t j = 0; j < skin->DeformerCount(); ++j) {
                FbxDeformer* deformer = skin->GetDeformer(j);
                FbxModel* mdl = fbxScene.GetByUid<FbxModel>(deformer->targetModel);
                FbxMatrix4 mat = pose->transforms[mdl->GetUid()];
                skeleton->AddBone(mdl->GetName(), *(gfxm::mat4*)&mat);
            }
            importData.skeletons[skin->GetUid()] = skeleton;
        }
        

        void* bufptr;
        size_t sz;
        mz_zip_writer_finalize_heap_archive(&zip, &bufptr, &sz);

        DataSourceRef data_ref(new DataSourceMemory((char*)bufptr, sz));
        std::shared_ptr<Mesh> mesh_ref(new Mesh());
        mesh_ref->Build(data_ref);
        mesh_ref->Name(MKSTR(geom->GetUid() << geom->GetName() << ".geo"));
        mesh_ref->Storage(Resource::LOCAL);
        importData.meshes[geom->GetUid()] = mesh_ref;

        /*
        GlobalDataRegistry().Add(
            MKSTR(geom->GetUid() << geom->GetName() << ".geo"),
            DataSourceRef(new DataSourceMemory((char*)bufptr, sz))
        );
        */
        mz_zip_writer_end(&zip);
    }

    
    auto _rename = [](const std::string& name) -> std::string {
        static std::map<std::string, std::string> nameMap = {
            { "T", "Translation" },
            { "R", "Rotation" },
            { "S", "Scale" },
            { "d|X", "x" },
            { "d|Y", "y" },
            { "d|Z", "z" }
        };
        if(nameMap.count(name)) return nameMap[name];
        return name;
    };

    for(unsigned i = 0; i < fbxScene.Count<FbxAnimationStack>(); ++i) {
        FbxAnimationStack* stack = fbxScene.Get<FbxAnimationStack>(i);
        mz_zip_archive zip;
        memset(&zip, 0, sizeof(zip));
        mz_zip_writer_init_heap(&zip, 0, 0);

        double fps = fbxScene.Settings().frameRate;
        mz_zip_writer_add_mem(&zip, "FrameRate", (void*)&fps, sizeof(fps), 0);
        double len = stack->Length();
        mz_zip_writer_add_mem(&zip, "Length", (void*)&len, sizeof(len), 0);

        for(unsigned j = 0; j < stack->LayerCount(); ++j) {
            FbxAnimationLayer* layer = stack->GetLayer(j);

            struct kf3_t {
                float frame;
                gfxm::vec3 value;
            };
            struct kf4_t {
                float frame;
                gfxm::vec4 value;
            };

            for(size_t k = 0; k < fbxScene.Count<FbxModel>(); ++k) {
                FbxModel* model = fbxScene.Get<FbxModel>(k);
                std::vector<kf3_t> tc;
                std::vector<kf4_t> rc;
                std::vector<kf3_t> sc;
                for(double f = 0.0; f < stack->Length(); f += 1.0f) {
                    FbxVector3 t = layer->EvalTranslation(*model, f);
                    FbxQuat r = layer->EvalRotation(*model, f);
                    FbxVector3 s = layer->EvalScale(*model, f);

                    tc.emplace_back(kf3_t{(float)f, gfxm::vec3(t.x, t.y, t.z)});
                    rc.emplace_back(kf4_t{(float)f, gfxm::vec4(r.x, r.y, r.z, r.w)});
                    sc.emplace_back(kf3_t{(float)f, gfxm::vec3(s.x, s.y, s.z)});
                }

                mz_zip_writer_add_mem(
                    &zip, 
                    MKSTR(
                        "Layers/" << j << "/" <<
                        model->GetName() << "/" << 
                        "Transform/" << 
                        "Translation.curve3").c_str(),
                    (void*)tc.data(), sizeof(kf3_t) * tc.size(), 0
                );
                mz_zip_writer_add_mem(
                    &zip, 
                    MKSTR(
                        "Layers/" << j << "/" <<
                        model->GetName() << "/" << 
                        "Transform/" << 
                        "Quaternion.curveq").c_str(),
                    (void*)rc.data(), sizeof(kf4_t) * rc.size(), 0
                );
                mz_zip_writer_add_mem(
                    &zip, 
                    MKSTR(
                        "Layers/" << j << "/" <<
                        model->GetName() << "/" << 
                        "Transform/" << 
                        "Scale.curve3").c_str(),
                    (void*)sc.data(), sizeof(kf3_t) * sc.size(), 0
                );
            }
        }

        void* bufptr;
        size_t sz;
        mz_zip_writer_finalize_heap_archive(&zip, &bufptr, &sz);
        
        DataSourceRef data_ref(new DataSourceMemory((char*)bufptr, sz));
        std::shared_ptr<Animation> anim_ref(new Animation());
        anim_ref->Build(data_ref);
        anim_ref->Name(MKSTR(stack->Name() << ".anim"));
        anim_ref->Storage(Resource::LOCAL);
        importData.anims[stack->GetUid()] = anim_ref;
        /*
        GlobalDataRegistry().Add(
            MKSTR(stack->Name() << ".anim"),
            DataSourceRef(new DataSourceMemory((char*)bufptr, sz))
        );
*/
        mz_zip_writer_end(&zip);
    }

    for(size_t i = 0; i < fbxScene.Count<FbxMaterial>(); ++i) {
        FbxMaterial* fbxMat = fbxScene.Get<FbxMaterial>(i);
        LOG("Loading fbx material: " << fbxMat->Name());
        FbxTexture* fbxTex = fbxMat->GetDiffuseTexture();
        if(fbxTex) {
            LOG("With diffuse texture: " << fbxTex->TextureName() << ", source: " << fbxTex->FileName());
        }
    }
}

inline void SceneFromFbxModel(FbxModel* fbxModel, FbxScene& fbxScene, SceneObject* sceneObject, FbxImportData& importData){
    importData.AddObject(fbxModel->GetUid(), sceneObject);

    for(unsigned i = 0; i < fbxModel->ChildCount(); ++i)
    {
        SceneObject* child = sceneObject->CreateObject();
        sceneObject->Get<Transform>()->Attach(child->Get<Transform>());

        SceneFromFbxModel(fbxModel->GetChild(i, fbxScene), fbxScene, child, importData);
    }

    sceneObject->Name(fbxModel->GetName());
    //LOG("Created object: " << fbxModel->GetName());
    //LOG("Type: " << fbxModel->GetType());

    sceneObject->Get<Transform>()->SetTransform(*(gfxm::mat4*)&fbxModel->GetTransform());
    if(fbxModel->GetType() == "Mesh") {
        FbxMesh* fbxMesh = fbxScene.GetByUid<FbxMesh>(fbxModel->GetUid());
        FbxGeometry* fbxGeometry = fbxScene.GetByUid<FbxGeometry>(fbxMesh->GetGeometryUid());
        
        if(importData.meshes.count(fbxMesh->GetGeometryUid())) {
            auto mesh_ref = importData.meshes[fbxMesh->GetGeometryUid()];
            sceneObject->Get<Model>()->mesh = mesh_ref;
        }

        FbxSkin* fbxSkin = fbxGeometry->GetSkin();
        if(fbxSkin) {
            auto skel = importData.skeletons[fbxSkin->GetUid()];
            resource_ref<Skeleton> skel_res_ref;
            skel_res_ref = skel;
            sceneObject->Get<Skin>()->SetSkeleton(skel_res_ref);
            FbxPose* bindPose = fbxScene.GetBindPose();
            if(bindPose) {
                sceneObject->Get<Skin>()->SetBindTransform(*(gfxm::mat4*)&bindPose->GetPose(fbxModel->GetUid()));
            }
            SceneObject* armatureRoot = sceneObject->Root()->FindObject(fbxSkin->Name());
            if(armatureRoot) {
                sceneObject->Get<Skin>()->SetArmatureRoot(
                    armatureRoot->WeakPtr()
                );
            } else {
                std::cout << "Failed to find armature root" << std::endl;
            }
        }
    }
    else if(fbxModel->GetType() == "LimbNode") {
        sceneObject->Get<Bone>();
        // TODO:
    }
    else if(fbxModel->GetType() == "Light") {
        // TODO: Different light types, parameters
        LightOmni* o = sceneObject->Get<LightOmni>();
        o->Color(
            1.0f,
            1.0f,
            1.0f
        );
        o->Intensity(1.0f);
    }
}

inline void SceneFromFbx(FbxScene& fbxScene, SceneObject* scene, FbxImportData& importData){
    for(unsigned i = 0; i < fbxScene.RootModelCount(); ++i)
    {
        SceneObject* child = scene->CreateObject();
        scene->Get<Transform>()->Attach(child->Get<Transform>());
        SceneFromFbxModel(fbxScene.GetRootModel(i), fbxScene, child, importData);
    }

    for(size_t i = 0; i < fbxScene.Count<FbxAnimationStack>(); ++i) {
        //auto animDriver = scene->Get<SkeletonAnimator>();
        auto animDriver = scene->Get<AnimationDriver>();
        auto stack = fbxScene.Get<FbxAnimationStack>(i);
        animDriver->AddAnim(stack->Name(), importData.anims[stack->GetUid()]);
        //animDriver->AddAnim(stack->Name(), MKSTR(stack->Name() << ".anim"));
    }
}

inline bool SceneFromFbx(const char* data, size_t size, SceneObject* scene)
{
    FbxScene* fbxScene = FbxScene::Create();
    if(!FbxReadMem(*fbxScene, data, size))
        return false;
    FbxImportData importData;
    ResourcesFromFbxScene(*fbxScene, importData);
    SceneFromFbx(*fbxScene, scene, importData);
    fbxScene->Destroy();
    return true;
}

inline bool SceneFromFbx(const std::string& filename, SceneObject* scene)
{    
    FbxScene* fbxScene = FbxScene::Create();
    if(!FbxReadFile(*fbxScene, filename))
        return false;
    FbxImportData importData;
    ResourcesFromFbxScene(*fbxScene, importData);
    SceneFromFbx(*fbxScene, scene, importData);
    fbxScene->_dumpFile(filename);
    fbxScene->Destroy();
    return true;
}

#endif
