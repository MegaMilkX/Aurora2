#ifndef MESH_DATA_H
#define MESH_DATA_H

#include <fstream>
#include <aurora/gfx.h>
#include <aurora/media/fbx.h>

#include <asset.h>

#include "../util/gl_helpers.h"

#include <util/scoped_timer.h>

#include "../util/gfxm.h"

struct GLAttribDesc
{
    std::string name;
    int elemCount;
    GLenum elemType;
    GLboolean normalized;
};

class Mesh
{
public:
    struct SubData
    {
        std::string name;
        unsigned int indexCount;
        unsigned int offset;
    };
    std::vector<SubData> subDataArray;
    
    template<typename ATTR, typename T>
    void SetAttribArray(const std::vector<T>& data);
    void SetIndices(const std::vector<unsigned>& data);
    template<typename T>
    unsigned GetAttribCount();
    template<typename T>
    T* GetAttribData();
    template<typename T>
    std::vector<unsigned char>& GetAttribBytes();
    std::vector<unsigned char>& GetAttribBytesByName(const std::string& name);
    std::vector<unsigned>& GetIndices();
    GLuint GetVao(const std::vector<GLAttribDesc>& vertexDesc);
    unsigned GetIndexCount();
    
    std::vector<GLVertexArrayObject> vertexArrayObjects;
    std::vector<bool> vaoDirty;
    std::map<Au::AttribInfo, std::vector<unsigned char>> attribArrays;
    std::vector<unsigned> indices;
    
    bool _compareDesc(
        const std::vector<GLAttribDesc>& vertexDesc,
        const GLVertexArrayObject& vao)
    {
        const std::vector<GLVertexBufferDesc>& vaoDesc =
            vao.Desc();
        if(vertexDesc.size() != vaoDesc.size())
            return false;
        for(unsigned i = 0; i < vertexDesc.size(); ++i)
        {
            const GLAttribDesc& d1 = vertexDesc[i];
            const GLVertexBufferDesc& d2 = vaoDesc[i];

            if(d1.name != d2.name ||
                d1.elemCount != d2.size ||
                d1.elemType != d2.type ||
                d1.normalized != d2.normalized)
            {
                return false;
            }
        }
        
        return true;
    }

public:
    bool Build(Resource* r)
    {
        if(!r) return false;

        std::vector<float> vertices;
        std::vector<float> normals;
        std::vector<float> uv;
        std::vector<unsigned> indices;
        std::vector<gfxm::vec4> boneIndices;
        std::vector<gfxm::vec4> boneWeights;
        unsigned int indexOffset = 0;

        for(auto kv : r->GetChildren())
        {
            std::string meshName = kv.first;
            
            int vertexCount = *(int*)kv.second->Get("VertexCount")->Data();
            int indexCount = *(int*)kv.second->Get("IndexCount")->Data();
            
            std::vector<float> v = 
                std::vector<float>( (float*)kv.second->Get("Vertices")->Data(), (float*)(kv.second->Get("Vertices")->Data() + kv.second->Get("Vertices")->DataSize()) );
            vertices.insert(vertices.end(), v.begin(), v.end());
            if(kv.second->Get("Normals"))
            {
                std::vector<float> n = 
                    std::vector<float>( (float*)kv.second->Get("Normals")->Get("0")->Data(), (float*)(kv.second->Get("Normals")->Get("0")->Data() + kv.second->Get("Normals")->Get("0")->DataSize()) );
                normals.insert(normals.end(), n.begin(), n.end());
            }
            else
            {
                normals.resize(vertices.size());
            }
            if(kv.second->Get("UV"))
            {
                std::vector<float> u = 
                    std::vector<float>( (float*)kv.second->Get("UV")->Get("0")->Data(), (float*)(kv.second->Get("UV")->Get("0")->Data() + kv.second->Get("UV")->Get("0")->DataSize()) );
                uv.insert(uv.end(), u.begin(), u.end());
            }
            else
            {
                uv.resize(vertices.size() / 3 * 2);
            }
            Mesh::SubData subData;
            subData.offset = indices.size() * sizeof(unsigned); // BYTE OFFSET
            
            std::vector<uint32_t> rawIndices = 
                std::vector<uint32_t>( (uint32_t*)kv.second->Get("Indices")->Data(), (uint32_t*)(kv.second->Get("Indices")->Data() + kv.second->Get("Indices")->DataSize()) );
            for(unsigned j = 0; j < rawIndices.size(); ++j)
                rawIndices[j] += indexOffset;
            indices.insert(indices.end(), rawIndices.begin(), rawIndices.end());
            
            subData.name = meshName;
            subData.indexCount = indexCount;
            subDataArray.push_back(subData);

            std::vector<gfxm::vec4> tmpBoneIndices;
            std::vector<gfxm::vec4> tmpBoneWeights;
            if(kv.second->Get("BoneIndices") && kv.second->Get("BoneWeights"))
            {
                tmpBoneIndices =
                    std::vector<gfxm::vec4>( (gfxm::vec4*)kv.second->Get("BoneIndices")->Data(), (gfxm::vec4*)(kv.second->Get("BoneIndices")->Data() + kv.second->Get("BoneIndices")->DataSize()) );
                tmpBoneWeights =
                    std::vector<gfxm::vec4>( (gfxm::vec4*)kv.second->Get("BoneWeights")->Data(), (gfxm::vec4*)(kv.second->Get("BoneWeights")->Data() + kv.second->Get("BoneWeights")->DataSize()) );
            }
            else
            {
                tmpBoneIndices.resize(vertices.size() / 3);
                tmpBoneWeights.resize(vertices.size() / 3);
            }
            boneIndices.insert(boneIndices.end(), tmpBoneIndices.begin(), tmpBoneIndices.end());
            boneWeights.insert(boneWeights.end(), tmpBoneWeights.begin(), tmpBoneWeights.end());
            
            indexOffset = vertices.size() / 3;
        }

        SetAttribArray<Au::Position>(vertices);
        SetAttribArray<Au::Normal>(normals);
        SetAttribArray<Au::UV>(uv);
        SetIndices(indices);
        if(boneIndices.size() > 0 && boneWeights.size() > 0)
        {
            SetAttribArray<Au::BoneIndex4>(boneIndices);
            SetAttribArray<Au::BoneWeight4>(boneWeights);
        }
        return true;
    }
};

template<typename ATTR, typename T>
void Mesh::SetAttribArray(const std::vector<T>& data)
{
    std::vector<unsigned char> bytes(
        (unsigned char*)data.data(), 
        (unsigned char*)data.data() + data.size() * sizeof(T)
    );
    
    attribArrays[ATTR()] = bytes;
    for(unsigned i = 0; i < vaoDirty.size(); ++i)
        vaoDirty[i] = true;
}

template<typename T>
unsigned Mesh::GetAttribCount()
{
    size_t attrSz = sizeof(T);
    if(attrSz == 0)
        return 0;
    unsigned count = attribArrays[T()].size() / attrSz;
    return count;
}

template<typename T>
T* Mesh::GetAttribData()
{
    return (T*)attribArrays[T()].data();
}

template<typename T>
std::vector<unsigned char>& Mesh::GetAttribBytes()
{
    return attribArrays[T()];
}

#endif
