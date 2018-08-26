#ifndef SCENE_FROM_FBX_H
#define SCENE_FROM_FBX_H

#include "fbx/fbx_read.h"
#define MINIZ_HEADER_FILE_ONLY
#include "../../lib/miniz.c"
#include <scene_object.h>

inline void ResourcesFromFbxScene(FbxScene& fbxScene)
{
    for(unsigned i = 0; i < fbxScene.GeometryCount(); ++i)
    {
        FbxGeometry& geom = fbxScene.GetGeometry(i);
        
        mz_zip_archive zip;
        memset(&zip, 0, sizeof(zip));
        mz_zip_writer_init_heap(&zip, 0, 0);

        int32_t vertexCount = (int32_t)geom.VertexCount();
        int32_t indexCount = (int32_t)geom.IndexCount();
        mz_zip_writer_add_mem(&zip, "VertexCount", (void*)&vertexCount, sizeof(vertexCount), 0);
        mz_zip_writer_add_mem(&zip, "IndexCount", (void*)&indexCount, sizeof(indexCount), 0);
        mz_zip_writer_add_mem(&zip, "Indices", (void*)geom.GetIndices().data(), indexCount * sizeof(uint32_t), 0);
        mz_zip_writer_add_mem(&zip, "Vertices", (void*)geom.GetVertices().data(), vertexCount * 3 * sizeof(float), 0);
        for(size_t j = 0; j < geom.NormalLayerCount(); ++j)
        {
            mz_zip_writer_add_mem(&zip, MKSTR("Normals." << j).c_str(), (void*)geom.GetNormals(0).data(), vertexCount * 3 * sizeof(float), 0);
        }
        for(size_t j = 0; j < geom.UVLayerCount(); ++j)
        {
            mz_zip_writer_add_mem(&zip, MKSTR("UV." << j).c_str(), (void*)geom.GetUV(0).data(), vertexCount * 2 * sizeof(float), 0);
        }

        void* bufptr;
        size_t sz;
        mz_zip_writer_finalize_heap_archive(&zip, &bufptr, &sz);

        g_resourceRegistry.Add(
            MKSTR(geom.GetUid() << geom.GetName() << ".geo"), 
            new ResourceRawMemory((char*)bufptr, sz)
        );
        mz_zip_writer_end(&zip);
    }
}

inline void SceneFromFbxModel(FbxModel& fbxModel, FbxScene& fbxScene, SceneObject* sceneObject){
    for(unsigned i = 0; i < fbxModel.ChildCount(); ++i)
    {
        SceneObject* child = sceneObject->CreateObject();
        sceneObject->Get<Transform>()->Attach(child->Get<Transform>());

        SceneFromFbxModel(fbxModel.GetChild(i, fbxScene), fbxScene, child);
    }

    sceneObject->Name(fbxModel.GetName());
    //LOG("Created object: " << fbxModel.GetName());
    //LOG("Type: " << fbxModel.GetType());

    sceneObject->Get<Transform>()->SetTransform(*(gfxm::mat4*)&fbxModel.GetTransform());
    if(fbxModel.GetType() == FbxMesh::Type())
    {
        FbxMesh& fbxMesh = fbxScene.GetMesh(fbxModel.GetUid());
        FbxGeometry& fbxGeometry = fbxScene.GetGeometryByUid(fbxMesh.GetGeometryUid());
        sceneObject->Get<Model>()->SetMesh(
            MKSTR(fbxGeometry.GetUid() << fbxGeometry.GetName() << ".geo")
        );

        auto& vertices = fbxGeometry.GetVertices();
        auto& indices = fbxGeometry.GetIndices();
    }
    else if(fbxModel.GetType() == FbxLight::Type())
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
    for(unsigned i = 0; i < fbxScene.ModelCount(); ++i)
    {
        SceneObject* child = scene->CreateObject();
        scene->Get<Transform>()->Attach(child->Get<Transform>());
        SceneFromFbxModel(fbxScene.GetModel(i), fbxScene, child);
    }
}

inline bool SceneFromFbx(const char* data, size_t size, SceneObject* scene)
{
    FbxScene fbxScene;
    if(!FbxReadMem(fbxScene, data, size))
        return false;
    ResourcesFromFbxScene(fbxScene);
    SceneFromFbx(fbxScene, scene);
    return true;
}

inline bool SceneFromFbx(const std::string& filename, SceneObject* scene)
{    
    FbxScene fbxScene;
    if(!FbxReadFile(fbxScene, filename))
        return false;
    ResourcesFromFbxScene(fbxScene);
    SceneFromFbx(fbxScene, scene);
    fbxScene._dumpFile(filename);
    return true;
}

#endif