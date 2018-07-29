#ifndef MESH_DATA_H
#define MESH_DATA_H

#include <fstream>
#include <aurora/gfx.h>
#include <aurora/media/fbx.h>

#include <asset.h>

#include "../util/gl_helpers.h"

#include <util/scoped_timer.h>

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
    /*
    void FillMesh(Au::GFX::Mesh* mesh)
    {
        std::vector<Au::AttribInfo> fmt = mesh->Format();
        
        for(unsigned i = 0; i < fmt.size(); ++i)
        {
            std::map<Au::AttribInfo, std::vector<unsigned char>>::iterator it =
                attribArrays.find(fmt[i]);
            if(it == attribArrays.end())
                continue;
            
            mesh->VertexAttribByInfo(fmt[i], it->second);
        }
        mesh->IndexData(indices);
        
        for(unsigned i = 0; i < subDataArray.size(); ++i)
        {
            SubData& subData = subDataArray[i];
            mesh->SetSubMesh(
                subData.name,
                subData.indexCount,
                subData.offset
            );
        }
    }
    */
    template<typename ATTR, typename T>
    void SetAttribArray(const std::vector<T>& data)
    {
        std::vector<unsigned char> bytes(
            (unsigned char*)data.data(), 
            (unsigned char*)data.data() + data.size() * sizeof(T)
        );
        
        attribArrays[ATTR()] = bytes;
        for(unsigned i = 0; i < vaoDirty.size(); ++i)
            vaoDirty[i] = true;
    }
    
    void SetIndices(const std::vector<unsigned>& data)
    { 
        indices = data;
        for(unsigned i = 0; i < vaoDirty.size(); ++i)
            vaoDirty[i] = true;
    }
    
    template<typename T>
    unsigned GetAttribCount()
    {
        size_t attrSz = sizeof(T);
        if(attrSz == 0)
            return 0;
        unsigned count = attribArrays[T()].size() / attrSz;
        return count;
    }
    
    template<typename T>
    T* GetAttribData()
    {
        return (T*)attribArrays[T()].data();
    }
    
    template<typename T>
    std::vector<unsigned char>& GetAttribBytes()
    {
        return attribArrays[T()];
    }
    
    std::vector<unsigned char>& GetAttribBytesByName(const std::string& name)
    {
        for(auto& kv : attribArrays)
        {
            if(kv.first.name == name)
                return kv.second;
        }
        return attribArrays[Au::Empty()];
    }
    
    std::vector<unsigned>& GetIndices()
    {
        return indices;
    }
    
    GLuint GetVao(const std::vector<GLAttribDesc>& vertexDesc)
    {
        for(unsigned i = 0; i < vertexArrayObjects.size(); ++i)
        {
            if(_compareDesc(vertexDesc, vertexArrayObjects[i]))
            {
                if(vaoDirty[i])
                {
                    vaoDirty.erase(vaoDirty.begin() + i);
                    vertexArrayObjects[i].Cleanup();
                    vertexArrayObjects.erase(vertexArrayObjects.begin() + i);
                    break;
                }
                else
                    return vertexArrayObjects[i].GetGlName();
            }
        }
        
        GLVertexArrayObject vao;
        std::vector<GLVertexBufferDesc> desc;
        for(const GLAttribDesc& d : vertexDesc)
        {
            desc.push_back({ 
                d.name, 
                d.elemCount, 
                d.elemType, d.normalized, 
                (GLsizei)(d.elemCount * glTypeSize(d.elemType)), 
                GL_STATIC_DRAW 
            });
        }
        vao.Init(desc);
        for(const GLAttribDesc& d : vertexDesc)
        {
            vao.FillArrayBuffer(d.name, GetAttribBytesByName(d.name));
        }
        vao.FillIndexBuffer(GetIndices());
        
        vertexArrayObjects.push_back(vao);
        vaoDirty.push_back(false);
        
        return vao.GetGlName();
    }
    unsigned GetIndexCount() 
    { return indices.size(); }
    
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

template<>
inline bool LoadAsset<Mesh, FBX>(Mesh* meshData, const std::string& filename)
{
    ScopedTimer timer("LoadAsset<Mesh, FBX>: '" + filename + "'");
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if(!file.is_open())
        return false;
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> buffer((unsigned int)size);
    if(file.read(buffer.data(), size))
    {
        Au::Media::FBX::Reader fbxReader;
        fbxReader.ReadMemory(buffer.data(), buffer.size());
        fbxReader.ConvertCoordSys(Au::Media::FBX::OPENGL);
        fbxReader.DumpFile(filename);
        
        int meshCount = fbxReader.MeshCount();
        std::vector<float> vertices;
        std::vector<float> normals;
        std::vector<float> uv;
        std::vector<unsigned> indices;
        std::vector<gfxm::vec4> boneIndices;
        std::vector<gfxm::vec4> boneWeights;
        unsigned int indexOffset = 0;
        for(int i = 0; i < meshCount; ++i)
        {
            Au::Media::FBX::Mesh& fbxMesh = fbxReader.GetMesh(i);
            int vertexCount = fbxMesh.VertexCount();
            
            std::vector<float> v = fbxMesh.GetVertices();
            vertices.insert(vertices.end(), v.begin(), v.end());
            std::vector<float> n = fbxMesh.GetNormals(0);
            normals.insert(normals.end(), n.begin(), n.end());
            std::vector<float> u = fbxMesh.GetUV(0);
            uv.insert(uv.end(), u.begin(), u.end());
            
            Mesh::SubData subData;
            subData.offset = indices.size() * sizeof(unsigned); // BYTE OFFSET
            
            std::vector<unsigned> rawIndices = fbxMesh.GetIndices<unsigned>();
            for(unsigned j = 0; j < rawIndices.size(); ++j)
                rawIndices[j] += indexOffset;
            indices.insert(indices.end(), rawIndices.begin(), rawIndices.end());
            
            subData.name = fbxMesh.name;
            subData.indexCount = rawIndices.size();
            meshData->subDataArray.push_back(subData);
            
            Au::Media::FBX::Skin skin = fbxMesh.GetSkin();
            std::vector<gfxm::vec4> tmpBoneIndices;
            std::vector<gfxm::vec4> tmpBoneWeights;
            std::vector<int> boneDataCount;
            tmpBoneIndices.resize(vertexCount, gfxm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
            tmpBoneWeights.resize(vertexCount, gfxm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
            boneDataCount.resize(vertexCount, 0);
            memset(boneDataCount.data(), 0, boneDataCount.size() * sizeof(int));
            for(unsigned j = 0; j < skin.BoneCount(); ++j)
            {
                int64_t uidBone = skin.GetBoneUID(j);
                Au::Media::FBX::Bone* bone = fbxReader.GetBoneByUID(uidBone);
                if(!bone)
                    continue;
                unsigned boneIndex = bone->Index();
                
                for(unsigned k = 0; k < bone->indices.size() && k < bone->weights.size(); ++k)
                {
                    int32_t vertexIndex = bone->indices[k];
                    float weight = bone->weights[k];
                    if(weight < 0.01f)
                        continue;
                    int& dataCount = boneDataCount[vertexIndex];
                    if(dataCount > 3)
                        continue;
                        
                    tmpBoneIndices[vertexIndex][dataCount] = (float)boneIndex;
                    tmpBoneWeights[vertexIndex][dataCount] = weight;
                    
                    dataCount++;
                }
            }
            
            //for(int j = 0; j < vertexCount; ++j)
            //    gfxm::normalize(tmpBoneWeights[j]);
            
            boneIndices.insert(boneIndices.end(), tmpBoneIndices.begin(), tmpBoneIndices.end());
            boneWeights.insert(boneWeights.end(), tmpBoneWeights.begin(), tmpBoneWeights.end());
            
            indexOffset = vertices.size() / 3;
        }

        meshData->SetAttribArray<Au::Position>(vertices);
        meshData->SetAttribArray<Au::Normal>(normals);
        meshData->SetAttribArray<Au::UV>(uv);
        meshData->SetIndices(indices);
        
        meshData->SetAttribArray<Au::BoneIndex4>(boneIndices);
        meshData->SetAttribArray<Au::BoneWeight4>(boneWeights);
    }
    file.close();
    return true;
}

#endif
