#include <iostream>
#include <string>

#include <windows.h>

#include "../../src/resource_compiler.h"
#include "../../../../lib/include/aurora/include/aurora/media/fbx.h"
#include "../../../../lib/include/gfxm.h"
#include <fstream>

#include "../../../general/util.h"

#include <algorithm>

struct keyframe
{
    float frame;
    float value;
};

bool MakeAnimation(ResourceCompiler* compiler, const char* name, Au::Media::FBX::Reader& fbxReader)
{
    auto stacks = fbxReader.GetAnimationStacks();
    double fps = fbxReader.GetFrameRate();
    double timePerFrame = Au::Media::FBX::TimeSecond / fps;

    //Bind poses
    for(unsigned i = 0; i < fbxReader.ModelCount(); ++i)
    {
        auto mdl = fbxReader.GetModel(i);
        std::string mdl_name = mdl->name;
        std::replace( mdl_name.begin(), mdl_name.end(), '.', ' ');

        compiler->SubmitMem(
            MKSTR(name << ".Bind" << "." << mdl_name).c_str(),
            (void*)&mdl->transform,
            sizeof(float) * 16
        );
    }

    compiler->SubmitMem(
        MKSTR(name << "." << ".FrameRate").c_str(),
        (void*)&fps, sizeof(double)
    );
    compiler->SubmitMem(
        MKSTR(name << "." << ".FrameTime").c_str(),
        (void*)&timePerFrame, sizeof(double)
    );

    for(unsigned i = 0; i < stacks.size(); ++i)
    {
        double length = stacks[i].GetLength() / timePerFrame;
        std::string animName = stacks[i].GetName();
        {
            // TODO: Check if fbx is made in blender, only then cut by first pipe symbol
            size_t pipe_pos = animName.find_first_of("|");
            if(pipe_pos != std::string::npos)
            {
                animName = animName.substr(pipe_pos + 1);
            }
        }
        std::replace( animName.begin(), animName.end(), '.', ' ');

        auto nodes = stacks[i].GetAnimatedNodes();
        for(unsigned j = 0; j < nodes.size(); ++j)
        {
            if(!stacks[i].HasPositionCurve(nodes[i]) &&
                !stacks[i].HasRotationCurve(nodes[i]) &&
                !stacks[i].HasScaleCurve(nodes[i]))
            {
                continue;
            }
            std::string nodeName = nodes[j].Name();
            std::replace( nodeName.begin(), nodeName.end(), '.', ' ');
            float frame = 0.0f;
            //std::cout << "  CurveNode " << nodeName << std::endl;
            std::vector<keyframe> frames_pos_x;
            std::vector<keyframe> frames_pos_y;
            std::vector<keyframe> frames_pos_z;
            std::vector<keyframe> frames_rot_x;
            std::vector<keyframe> frames_rot_y;
            std::vector<keyframe> frames_rot_z;
            std::vector<keyframe> frames_rot_w;
            std::vector<keyframe> frames_scl_x;
            std::vector<keyframe> frames_scl_y;
            std::vector<keyframe> frames_scl_z;
            for(double t = 0.0f; t < length * timePerFrame; t += timePerFrame)
            {
                gfxm::vec3 pos = 
                    *(gfxm::vec3*)&stacks[i].EvaluatePosition(nodes[j], (int64_t)t);
                frames_pos_x.emplace_back(keyframe{frame, pos.x});
                frames_pos_y.emplace_back(keyframe{frame, pos.y});
                frames_pos_z.emplace_back(keyframe{frame, pos.z});

                gfxm::quat rot = 
                    *(gfxm::quat*)&stacks[i].EvaluateRotation(nodes[j], (int64_t)t);
                frames_rot_x.emplace_back(keyframe{frame, rot.x});
                frames_rot_y.emplace_back(keyframe{frame, rot.y});
                frames_rot_z.emplace_back(keyframe{frame, rot.z});
                frames_rot_w.emplace_back(keyframe{frame, rot.w});
                
                gfxm::vec3 scale = 
                    *(gfxm::vec3*)&stacks[i].EvaluateScale(nodes[j], (int64_t)t);
                frames_scl_x.emplace_back(keyframe{frame, scale.x});
                frames_scl_y.emplace_back(keyframe{frame, scale.y});
                frames_scl_z.emplace_back(keyframe{frame, scale.z});
                
                frame += 1.0f;
            }

            compiler->SubmitMem(
                MKSTR(name << ".Track" << "." << animName << "." << nodeName << ".Pos.X").c_str(),
                (void*)frames_pos_x.data(), sizeof(keyframe) * frames_pos_x.size()
            );
            compiler->SubmitMem(
                MKSTR(name << ".Track" << "." << animName << "."  << nodeName << ".Pos.Y").c_str(),
                (void*)frames_pos_y.data(), sizeof(keyframe) * frames_pos_y.size()
            );
            compiler->SubmitMem(
                MKSTR(name << ".Track" << "." << animName << "."  << nodeName << ".Pos.Z").c_str(),
                (void*)frames_pos_z.data(), sizeof(keyframe) * frames_pos_z.size()
            );

            compiler->SubmitMem(
                MKSTR(name << ".Track" << "." << animName << "." << nodeName << ".Rot.X").c_str(),
                (void*)frames_rot_x.data(), sizeof(keyframe) * frames_rot_x.size()
            );
            compiler->SubmitMem(
                MKSTR(name << ".Track" << "." << animName << "."  << nodeName << ".Rot.Y").c_str(),
                (void*)frames_rot_y.data(), sizeof(keyframe) * frames_rot_y.size()
            );
            compiler->SubmitMem(
                MKSTR(name << ".Track" << "." << animName << "."  << nodeName << ".Rot.Z").c_str(),
                (void*)frames_rot_z.data(), sizeof(keyframe) * frames_rot_z.size()
            );
            compiler->SubmitMem(
                MKSTR(name << ".Track" << "." << animName << "."  << nodeName << ".Rot.W").c_str(),
                (void*)frames_rot_w.data(), sizeof(keyframe) * frames_rot_w.size()
            );

            compiler->SubmitMem(
                MKSTR(name << ".Track" << "." << animName << "." << nodeName << ".Scl.X").c_str(),
                (void*)frames_scl_x.data(), sizeof(keyframe) * frames_scl_x.size()
            );
            compiler->SubmitMem(
                MKSTR(name << ".Track" << "." << animName << "."  << nodeName << ".Scl.Y").c_str(),
                (void*)frames_scl_y.data(), sizeof(keyframe) * frames_scl_y.size()
            );
            compiler->SubmitMem(
                MKSTR(name << ".Track" << "." << animName << "."  << nodeName << ".Scl.Z").c_str(),
                (void*)frames_scl_z.data(), sizeof(keyframe) * frames_scl_z.size()
            );
        }
    }

    return true;
}

bool MakeSkeleton(ResourceCompiler* compiler, const char* name, Au::Media::FBX::Reader& fbxReader)
{
    for(unsigned i = 0; i < fbxReader.ModelCount(); ++i)
    {
        Au::Media::FBX::Model* fbxModel = 
            fbxReader.GetModel(i);
        Au::Media::FBX::Model* fbxParentModel = 
            fbxReader.GetModelByUID(fbxModel->parentUID);
        std::string parentName = fbxParentModel ? fbxParentModel->name : "";
        std::string boneName = fbxModel->name;
        std::string boneNameOrig = boneName;
        std::replace( boneName.begin(), boneName.end(), '.', ' ');

        compiler->SubmitMem(MKSTR(name << "." << boneName << ".Name").c_str(), (void*)boneNameOrig.data(), boneNameOrig.size());
        compiler->SubmitMem(MKSTR(name << "." << boneName << ".Transform").c_str(), (void*)&fbxModel->transform, sizeof(float) * 4 * 4);
        compiler->SubmitMem(MKSTR(name << "." << boneName << ".Parent").c_str(), (void*)parentName.data(), parentName.size());
    }
    return true;
}

bool MakeModel(ResourceCompiler* compiler, const char* name, Au::Media::FBX::Reader& fbxReader)
{
    for(int i = 0; i < fbxReader.MeshCount(); ++i)
    {
        Au::Media::FBX::Mesh& mesh = fbxReader.GetMesh(i);

        std::string mname = mesh.name;
        std::replace( mname.begin(), mname.end(), '.', ' ');

        int vertexCount = mesh.VertexCount();
        compiler->SubmitMem(MKSTR(name << "." << mname << "." << "VertexCount").c_str(), (void*)&vertexCount, sizeof(int));
        
        auto indices = mesh.GetIndices<uint32_t>();
        int indexCount = indices.size();
        compiler->SubmitMem(MKSTR(name << "." << mname << "." << "IndexCount").c_str(), (void*)&indexCount, sizeof(indexCount));
        compiler->SubmitMem(MKSTR(name << "." << mname << "." << "Indices").c_str(), (void*)indices.data(), sizeof(uint32_t) * indices.size());

        auto vertices = mesh.GetVertices();
        compiler->SubmitMem(MKSTR(name << "." << mname << "." << "Vertices").c_str(), (void*)vertices.data(), sizeof(float) * vertices.size());

        for(unsigned l = 0; l < mesh.normalLayers.size(); ++l)
        {
            auto normals = mesh.GetNormals(l);
            compiler->SubmitMem(MKSTR(name << "." << mname << "." << "Normals." << l).c_str(), (void*)normals.data(), sizeof(float) * normals.size());
        }

        for(unsigned l = 0; l < mesh.uvLayers.size(); ++l)
        {
            auto uvs = mesh.GetUV(l);
            compiler->SubmitMem(MKSTR(name << "." << mname << "." << "UV." << l).c_str(), (void*)uvs.data(), sizeof(float) * uvs.size());
        }

        Au::Media::FBX::Skin& skin = mesh.GetSkin();
        if(skin.BoneCount() > 0)
        {
            std::vector<gfxm::vec4> BoneIndices;
            std::vector<gfxm::vec4> BoneWeights;
            std::vector<int> boneDataCount;
            BoneIndices.resize(vertexCount, gfxm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
            BoneWeights.resize(vertexCount, gfxm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
            boneDataCount.resize(vertexCount, 0);
            for(unsigned j = 0; j < skin.BoneCount(); ++j)
            {
                int64_t uidBode = skin.GetBoneUID(j);
                Au::Media::FBX::Bone* bone =
                    fbxReader.GetBoneByUID(uidBode);
                if(!bone) continue;
                unsigned boneIndex = bone->Index();

                for(unsigned k = 0; k < bone->indices.size() && k < bone->weights.size(); ++k)
                {
                    int32_t vertexIndex = bone->indices[k];
                    float weight = bone->weights[k];
                    if(weight < 0.01f) continue;
                    int& dataCount = boneDataCount[vertexIndex];
                    if(dataCount > 3) continue;

                    BoneIndices[vertexIndex][dataCount] = (float)boneIndex;
                    BoneWeights[vertexIndex][dataCount] = weight;
                    
                    dataCount++;
                }
            }
            compiler->SubmitMem(MKSTR(name << "." << mname << "." << "BoneIndices").c_str(), (void*)BoneIndices.data(), sizeof(gfxm::vec4) * BoneIndices.size());
            compiler->SubmitMem(MKSTR(name << "." << mname << "." << "BoneWeights").c_str(), (void*)BoneWeights.data(), sizeof(gfxm::vec4) * BoneWeights.size());
        }
    }

    return true;
}

bool MakeResource(ResourceCompiler* compiler, const char* name, const char* type, const char* source_filename)
{
    if(std::string(type) != "Model" &&
        std::string(type) != "Skeleton" &&
        std::string(type) != "Animation")
    {
        return false;
    }

    std::ifstream f(source_filename, std::ios::binary | std::ios::ate);
    if(!f.is_open())
    {
        return false;
    }
    std::streamsize size = f.tellg();
    f.seekg(0, std::ios::beg);
    std::vector<char> buffer((unsigned int)size);
    if(!f.read(buffer.data(), size))
    {
        f.close();
        return false;
    }

    Au::Media::FBX::Reader fbxReader;
    if(!fbxReader.ReadMemory(buffer.data(), (unsigned)size))
    {
        LOG_ERR("Failed to load fbx: " << source_filename);
        return false;
    }

    bool result = false;
    
    if(std::string(type) == "Model")
    {
        result = MakeModel(compiler, name, fbxReader);
    }
    else if(std::string(type) == "Skeleton")
    {
        result = MakeSkeleton(compiler, name, fbxReader);
    }
    else if(std::string(type) == "Animation")
    {
        result = MakeAnimation(compiler, name, fbxReader);
    }

    f.close();

    return result;
}