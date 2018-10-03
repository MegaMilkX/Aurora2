#ifndef SCENE_FROM_FBX_H
#define SCENE_FROM_FBX_H

#include "fbx_/fbx_scene.h"

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

    void Root(SceneObject* so) { root = so; }
    SceneObject* Root() { return root; }
private:
    SceneObject* root;
    std::vector<SceneObject*> loadedObjects;
    std::map<int64_t, SceneObject*> fbxUidToObject;
};

inline void ResourcesFromFbxScene(Fbx::Scene& fbxScene, FbxImportData& importData)
{
    for(unsigned i = 0; i < fbxScene.Objects().Count<Fbx::Geometry>(); ++i)
    {
        Fbx::Geometry* geom = fbxScene.Objects().Get<Fbx::Geometry>(i);
        Fbx::IndexedTriangleMesh fbxMesh = geom->MakeIndexedMesh();
        
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
        
/*
        FbxSkin* skin = geom->GetSkin();
        FbxPose* pose = fbxScene.GetBindPose();
        if(skin) {
            std::shared_ptr<Skeleton> skeleton(new Skeleton());
            skeleton->Name(MKSTR(skin->GetUid() << ".skel"));
            skeleton->Storage(Resource::LOCAL);
            for(size_t j = 0; j < skin->DeformerCount(); ++j) {
                FbxDeformer* deformer = skin->GetDeformer(j);
                FbxModel* mdl = fbxScene.GetByUid<FbxModel>(deformer->targetModel);
                FbxMatrix4 mat;
                if(pose) {
                    mat = pose->transforms[mdl->GetUid()];
                } else {
                    mat = deformer->transformLink;
                }
                skeleton->AddBone(mdl->GetName(), *(gfxm::mat4*)&mat);
            }
            importData.skeletons[skin->GetUid()] = skeleton;
            LOG("Built skeleton");
            skin->Print();
        }
        */

        void* bufptr;
        size_t sz;
        mz_zip_writer_finalize_heap_archive(&zip, &bufptr, &sz);

        DataSourceRef data_ref(new DataSourceMemory((char*)bufptr, sz));
        std::shared_ptr<Mesh> mesh_ref(new Mesh());
        mesh_ref->Build(data_ref);
        mesh_ref->Name(MKSTR(geom->GetUid() << geom->GetName() << ".geo"));
        mesh_ref->Storage(Resource::LOCAL);
        importData.meshes[geom->GetUid()] = mesh_ref;

        mz_zip_writer_end(&zip);
    }

    for(size_t i = 0; i < fbxScene.Objects().Count<Fbx::DeformerSkin>(); ++i) {
        Fbx::DeformerSkin* skin = fbxScene.Objects().Get<Fbx::DeformerSkin>(i);
        std::shared_ptr<Skeleton> skeleton(new Skeleton());
        skeleton->Name(MKSTR(skin->GetUid() << ".skel"));
        skeleton->Storage(Resource::LOCAL);
        size_t cluster_count = 
            fbxScene.CountChildren<Fbx::DeformerCluster>(Fbx::OBJECT_OBJECT, skin->GetUid());
        for(size_t j = 0; j < cluster_count; ++j) {
            Fbx::DeformerCluster* cluster = 
                fbxScene.GetChild<Fbx::DeformerCluster>(Fbx::OBJECT_OBJECT, skin->GetUid());
            Fbx::Model* model = 
                fbxScene.GetChild<Fbx::Model>(Fbx::OBJECT_OBJECT, cluster->GetUid());
            if(!model) continue;
            skeleton->AddBone(model->GetName(), *(gfxm::mat4*)&cluster->transformLink);
        }
        importData.skeletons[skin->GetUid()] = skeleton;
    }
/*
    
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
    */
}

inline void SceneFromFbxModel(Fbx::Model* fbxModel, Fbx::Scene& fbxScene, SceneObject* sceneObject, FbxImportData& importData){
    importData.AddObject(fbxModel->GetUid(), sceneObject);

    size_t child_count = fbxScene.CountChildren<Fbx::Model>(Fbx::OBJECT_OBJECT, fbxModel->GetUid());
    for(unsigned i = 0; i < child_count; ++i)
    {
        SceneObject* child = sceneObject->CreateObject();
        sceneObject->Get<Transform>()->Attach(child->Get<Transform>());

        Fbx::Model* child_model =
            fbxScene.GetChild<Fbx::Model>(Fbx::OBJECT_OBJECT, fbxModel->GetUid(), i);

        SceneFromFbxModel(
            child_model,
            fbxScene, 
            child, 
            importData
        );
    }

    sceneObject->Name(fbxModel->GetName());
    //LOG("Created object: " << fbxModel->GetName());
    //LOG("Type: " << fbxModel->GetType());
    sceneObject->Get<Transform>()->SetTransform(*(gfxm::mat4*)&fbxModel->lclTransform);

    Fbx::Geometry* geom = 0;
    if(geom = fbxScene.GetChild<Fbx::Geometry>(Fbx::OBJECT_OBJECT, fbxModel->GetUid())) {
        auto it = importData.meshes.find(geom->GetUid());
        if(it != importData.meshes.end()) {
            sceneObject->Get<Model>()->mesh = it->second;
        }

        /*
        Fbx::DeformerSkin* fbxSkin = fbxScene.GetChild<Fbx::DeformerSkin>(Fbx::OBJECT_OBJECT, geom->GetUid());
        if(fbxSkin) {
            LOG("Found skin!");
            Skin* skin = sceneObject->Get<Skin>();
            skin->SetArmatureRoot(importData.Root()->WeakPtr());
            auto it = importData.skeletons.find(fbxSkin->GetUid());
            if(it != importData.skeletons.end()) {
                resource_ref<Skeleton> skel_res_ref;
                skel_res_ref = it->second;
                skin->SetSkeleton(skel_res_ref);
            }
        }
        */
    }
    /*
    if(fbxModel->GetType() == "Mesh") {
        FbxMesh* fbxMesh = fbxScene.GetByUid<FbxMesh>(fbxModel->GetUid());
        FbxGeometry* fbxGeometry = fbxScene.GetByUid<FbxGeometry>(fbxMesh->GetGeometryUid());
        
        if(importData.meshes.count(fbxMesh->GetGeometryUid())) {
            auto mesh_ref = importData.meshes[fbxMesh->GetGeometryUid()];
            sceneObject->Get<Model>()->mesh = mesh_ref;
        }

        FbxSkin* fbxSkin = fbxGeometry->GetSkin();
        if(fbxSkin) {
            LOG("SceneFromFbx: " << sceneObject->Name() << " model has skin");
            if(importData.skeletons.find(fbxSkin->GetUid()) != importData.skeletons.end()) {
                LOG("Found skeleton");
                auto skel = importData.skeletons[fbxSkin->GetUid()];
                resource_ref<Skeleton> skel_res_ref;
                skel_res_ref = skel;
                sceneObject->Get<Skin>()->SetSkeleton(skel_res_ref);
            } else {
                LOG("Skeleton not found");
            }
            FbxPose* bindPose = fbxScene.GetBindPose();
            if(bindPose) {
                LOG("Bind pose found");
                sceneObject->Get<Skin>()->SetBindTransform(*(gfxm::mat4*)&bindPose->GetPose(fbxModel->GetUid()));
            } else {
                LOG("No bind pose for skin mesh");
                if(fbxSkin->DeformerCount() > 0) {
                    FbxDeformer* deformer = fbxSkin->GetDeformer(0);
                    sceneObject->Get<Skin>()->SetBindTransform(*(gfxm::mat4*)&deformer->transform);
                } else {
                    sceneObject->Get<Skin>()->SetBindTransform(*(gfxm::mat4*)&fbxModel->GetTransform());
                }
            }
            SceneObject* armatureRoot = importData.Root();
            if(armatureRoot) {
                LOG("Armature root is present");
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
    */
}

inline void SceneFromFbx(Fbx::Scene& fbxScene, SceneObject* scene, FbxImportData& importData){
    for(unsigned i = 0; i < fbxScene.RootModelCount(); ++i)
    {
        SceneObject* child = scene->CreateObject();
        scene->Get<Transform>()->Attach(child->Get<Transform>());
        SceneFromFbxModel(fbxScene.GetRootModel(i), fbxScene, child, importData);
    }

    float scaleFactor = 1.0f;
    if(fbxScene.properties) {
        double scaleFactorD = fbxScene.properties->GetValue<double>("UnitScaleFactor");
        if(scaleFactorD != 0.0) {
            scaleFactor = (float)(scaleFactorD * 0.01);
        }
    }

    scene->Get<Transform>()->Scale(gfxm::vec3(1.0f, 1.0f, 1.0f) * scaleFactor);

    /*
    for(size_t i = 0; i < fbxScene.Count<FbxAnimationStack>(); ++i) {
        //auto animDriver = scene->Get<SkeletonAnimator>();
        auto animDriver = scene->Get<AnimationDriver>();
        auto stack = fbxScene.Get<FbxAnimationStack>(i);
        animDriver->AddAnim(stack->Name(), importData.anims[stack->GetUid()]);
        //animDriver->AddAnim(stack->Name(), MKSTR(stack->Name() << ".anim"));
    }
    */
}

inline bool SceneFromFbx(const char* data, size_t size, SceneObject* scene)
{
    Fbx::SetLogCallback([](const std::string& s){
        LOG("FBX: " << s);
    });
    Fbx::SetLogWarnCallback([](const std::string& s){
        LOG("FBX: " << s);
    });
    Fbx::SetLogErrCallback([](const std::string& s){
        LOG("FBX: " << s);
    });
    Fbx::SetLogDbgCallback([](const std::string& s){
        LOG("FBX: " << s);
    });

    FbxImportData importData;
    importData.Root(scene);

    Fbx::Scene _fbxScene;
    if(!_fbxScene.ReadMem(data, size)) {
        return false;
    }
    ResourcesFromFbxScene(_fbxScene, importData);
    SceneFromFbx(_fbxScene, scene, importData);

    /*
    FbxScene* fbxScene = FbxScene::Create();
    if(!FbxReadMem(*fbxScene, data, size))
        return false;
    FbxImportData importData;
    importData.Root(scene);

    ResourcesFromFbxScene(*fbxScene, importData);
    SceneFromFbx(*fbxScene, scene, importData);
    fbxScene->Destroy();
    */
    return true;
}

inline bool SceneFromFbx(const std::string& filename, SceneObject* scene)
{
    Fbx::SetLogCallback([](const std::string& s){
        LOG("FBX: " << s);
    });
    Fbx::SetLogWarnCallback([](const std::string& s){
        LOG("FBX: " << s);
    });
    Fbx::SetLogErrCallback([](const std::string& s){
        LOG("FBX: " << s);
    });
    Fbx::SetLogDbgCallback([](const std::string& s){
        LOG("FBX: " << s);
    });

    FbxImportData importData;
    importData.Root(scene);

    Fbx::Scene _fbxScene;
    if(!_fbxScene.ReadFile(filename)) {
        return false;
    }
    ResourcesFromFbxScene(_fbxScene, importData);
    SceneFromFbx(_fbxScene, scene, importData);
    _fbxScene._dumpFile(filename);

    /*
    FbxScene* fbxScene = FbxScene::Create();
    if(!FbxReadFile(*fbxScene, filename))
        return false;
    FbxImportData importData;
    importData.Root(scene);

    ResourcesFromFbxScene(*fbxScene, importData);
    SceneFromFbx(*fbxScene, scene, importData);
    fbxScene->_dumpFile(filename);
    fbxScene->Destroy();
    */
    return true;
}

#endif
