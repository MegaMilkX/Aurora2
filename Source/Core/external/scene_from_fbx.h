#ifndef SCENE_FROM_FBX_H
#define SCENE_FROM_FBX_H

#include "fbx/fbx_read.h"
#define MINIZ_HEADER_FILE_ONLY
#include "../../lib/miniz.c"
#include <scene_object.h>

#include <animation_driver.h>

inline void ResourcesFromFbxScene(FbxScene& fbxScene)
{
    for(unsigned i = 0; i < fbxScene.Count<FbxGeometry>(); ++i)
    {
        FbxGeometry* geom = fbxScene.Get<FbxGeometry>(i);
        
        mz_zip_archive zip;
        memset(&zip, 0, sizeof(zip));
        mz_zip_writer_init_heap(&zip, 0, 0);

        int32_t vertexCount = (int32_t)geom->VertexCount();
        int32_t indexCount = (int32_t)geom->IndexCount();
        mz_zip_writer_add_mem(&zip, "VertexCount", (void*)&vertexCount, sizeof(vertexCount), 0);
        mz_zip_writer_add_mem(&zip, "IndexCount", (void*)&indexCount, sizeof(indexCount), 0);
        mz_zip_writer_add_mem(&zip, "Indices", (void*)geom->GetIndices().data(), indexCount * sizeof(uint32_t), 0);
        mz_zip_writer_add_mem(&zip, "Vertices", (void*)geom->GetVertices().data(), vertexCount * 3 * sizeof(float), 0);
        for(size_t j = 0; j < geom->NormalLayerCount(); ++j)
        {
            mz_zip_writer_add_mem(&zip, MKSTR("Normals." << j).c_str(), (void*)geom->GetNormals(0).data(), vertexCount * 3 * sizeof(float), 0);
        }
        for(size_t j = 0; j < geom->UVLayerCount(); ++j)
        {
            mz_zip_writer_add_mem(&zip, MKSTR("UV." << j).c_str(), (void*)geom->GetUV(0).data(), vertexCount * 2 * sizeof(float), 0);
        }

        void* bufptr;
        size_t sz;
        mz_zip_writer_finalize_heap_archive(&zip, &bufptr, &sz);

        g_resourceRegistry.Add(
            MKSTR(geom->GetUid() << geom->GetName() << ".geo"), 
            new ResourceRawMemory((char*)bufptr, sz)
        );
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
            for(unsigned k = 0; k < layer->CurveNodeCount(); ++k) {
                FbxAnimationCurveNode* curveNode = layer->GetCurveNode(k);
                for(unsigned l = 0; l < curveNode->CurveCount(); ++l) {
                    auto curv = curveNode->GetCurve(l);

                    auto& kf = curv->GetKeyframes();
                    struct _keyframe {
                        float time;
                        float value;
                    };
                    std::vector<_keyframe> frames;
                    for(auto& f : kf) {
                        double timePerFrame = FbxTimeSecond / fbxScene.Settings().frameRate;
                        float keyTime = f.frame / timePerFrame;
                        frames.emplace_back(_keyframe{keyTime, f.value});
                    }

                    mz_zip_writer_add_mem(
                        &zip, 
                        MKSTR(
                            "Layers/" << j << "/" <<
                            curveNode->OwnerName() << "/" << 
                            "Transform/" <<
                            _rename(curveNode->Name()) << "/" << 
                            _rename(curv->Name())).c_str(),
                        (void*)frames.data(), sizeof(_keyframe) * frames.size(), 0
                    );
                }
            }
        }

        void* bufptr;
        size_t sz;
        mz_zip_writer_finalize_heap_archive(&zip, &bufptr, &sz);
        g_resourceRegistry.Add(
            MKSTR(stack->Name() << ".anim"), 
            new ResourceRawMemory((char*)bufptr, sz)
        );
        mz_zip_writer_end(&zip);
    }
}

inline void SceneFromFbxModel(FbxModel* fbxModel, FbxScene& fbxScene, SceneObject* sceneObject){
    for(unsigned i = 0; i < fbxModel->ChildCount(); ++i)
    {
        SceneObject* child = sceneObject->CreateObject();
        sceneObject->Get<Transform>()->Attach(child->Get<Transform>());

        SceneFromFbxModel(fbxModel->GetChild(i, fbxScene), fbxScene, child);
    }

    sceneObject->Name(fbxModel->GetName());
    //LOG("Created object: " << fbxModel->GetName());
    //LOG("Type: " << fbxModel->GetType());

    sceneObject->Get<Transform>()->SetTransform(*(gfxm::mat4*)&fbxModel->GetTransform());
    if(fbxModel->GetType() == "Mesh")
    {
        FbxMesh* fbxMesh = fbxScene.GetByUid<FbxMesh>(fbxModel->GetUid());
        FbxGeometry* fbxGeometry = fbxScene.GetByUid<FbxGeometry>(fbxMesh->GetGeometryUid());
        sceneObject->Get<Model>()->SetMesh(
            MKSTR(fbxGeometry->GetUid() << fbxGeometry->GetName() << ".geo")
        );

        auto& vertices = fbxGeometry->GetVertices();
        auto& indices = fbxGeometry->GetIndices();
    }
    else if(fbxModel->GetType() == "Light")
    {
        LightOmni* o = sceneObject->Get<LightOmni>();
        o->Color(
            1.0f,
            1.0f,
            1.0f
            /*((rand() % 50 + 50) * 0.01f), 
            ((rand() % 50 + 50) * 0.01f), 
            ((rand() % 50 + 50) * 0.01f)*/
        );
        o->Intensity(1.0f);
    }
}

inline void SceneFromFbx(FbxScene& fbxScene, SceneObject* scene){
    for(unsigned i = 0; i < fbxScene.RootModelCount(); ++i)
    {
        SceneObject* child = scene->CreateObject();
        scene->Get<Transform>()->Attach(child->Get<Transform>());
        SceneFromFbxModel(fbxScene.GetRootModel(i), fbxScene, child);
    }

    if(fbxScene.Count<FbxAnimationStack>() > 0) {
        auto animDriver = scene->Get<AnimationDriver>();
        auto stack = fbxScene.Get<FbxAnimationStack>(0);
        animDriver->AddAnim(MKSTR(stack->Name() << ".anim"), MKSTR(stack->Name() << ".anim"));
    }
}

inline bool SceneFromFbx(const char* data, size_t size, SceneObject* scene)
{
    FbxScene* fbxScene = FbxScene::Create();
    if(!FbxReadMem(*fbxScene, data, size))
        return false;
    ResourcesFromFbxScene(*fbxScene);
    SceneFromFbx(*fbxScene, scene);
    fbxScene->Destroy();
    return true;
}

inline bool SceneFromFbx(const std::string& filename, SceneObject* scene)
{    
    FbxScene* fbxScene = FbxScene::Create();
    if(!FbxReadFile(*fbxScene, filename))
        return false;
    ResourcesFromFbxScene(*fbxScene);
    SceneFromFbx(*fbxScene, scene);
    fbxScene->_dumpFile(filename);
    fbxScene->Destroy();
    return true;
}

#endif
