#ifndef MESH_DATA_H
#define MESH_DATA_H

#include <fstream>
#include <aurora/gfx.h>

#include "../util/gl_helpers.h"

#include <util/scoped_timer.h>

#include "../util/gfxm.h"

#include "resource/resource.h"

#include "../../general/util.h"

struct GLAttribDesc
{
    std::string name;
    int elemCount;
    GLenum elemType;
    GLboolean normalized;
};

class Mesh : public Resource
{
    RTTR_ENABLE(Resource)
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
    void SetIndices(const std::vector<uint32_t>& data);
    template<typename T>
    unsigned GetAttribCount();
    template<typename T>
    T* GetAttribData();
    template<typename T>
    std::vector<unsigned char>& GetAttribBytes();
    std::vector<unsigned char>& GetAttribBytesByName(const std::string& name);
    std::vector<unsigned>& GetIndices();
    GLuint GetVao();
    GLuint GetSkinVao();
    unsigned GetIndexCount();
    
    //std::vector<GLVertexArrayObject> vertexArrayObjects;
    //std::vector<bool> vaoDirty;
    GLVertexArrayObject vertexArrayObject;
    bool vaoDirty = true;
    GLVertexArrayObject vertexArrayObjectSkin;
    bool vaoSkinDirty = true;

    std::map<Au::AttribInfo, std::vector<unsigned char>> attribArrays;
    std::vector<uint32_t> indices;
    int vertexCount;
    
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
    bool Build(DataSourceRef r)
    {
        if(r->Size() == 0) return false;
        std::vector<char> buffer;
        buffer.resize((size_t)r->Size());
        r->ReadAll((char*)buffer.data());

        mz_zip_archive zip;
        memset(&zip, 0, sizeof(zip));
        if(!mz_zip_reader_init_mem(&zip, buffer.data(), buffer.size(), 0))
        {
            //LOG_ERR("Failed to build mesh");
            return false;
        }

        vertexCount = 0;
        std::vector<float> vertices;
        std::vector<float> normals;
        std::vector<float> uv;
        std::vector<uint32_t> indices;
        std::vector<int32_t> boneIndices4;
        std::vector<float> boneWeights4;
        unsigned int indexOffset = 0;

        int index = 0;
        mz_zip_archive_file_stat file_stat;

        mz_zip_reader_extract_file_to_mem(&zip, "VertexCount", (void*)&vertexCount, (size_t)sizeof(vertexCount), 0);

        index = mz_zip_reader_locate_file(&zip, "Vertices", "", 0);
        mz_zip_reader_file_stat(&zip, index, &file_stat);
        vertices.resize((size_t)file_stat.m_uncomp_size / sizeof(float));
        mz_zip_reader_extract_file_to_mem(&zip, "Vertices", (void*)vertices.data(), (size_t)file_stat.m_uncomp_size, 0);

        index = mz_zip_reader_locate_file(&zip, "Indices", "", 0);
        mz_zip_reader_file_stat(&zip, index, &file_stat);
        indices.resize((size_t)file_stat.m_uncomp_size / sizeof(uint32_t));
        mz_zip_reader_extract_file_to_mem(&zip, "Indices", (void*)indices.data(), (size_t)file_stat.m_uncomp_size, 0);

        index = mz_zip_reader_locate_file(&zip, "Normals.0", "", 0);
        mz_zip_reader_file_stat(&zip, index, &file_stat);
        normals.resize((size_t)file_stat.m_uncomp_size / sizeof(float));
        mz_zip_reader_extract_file_to_mem(&zip, "Normals.0", (void*)normals.data(), (size_t)file_stat.m_uncomp_size, 0);
        
        index = mz_zip_reader_locate_file(&zip, "UV.0", "", 0);
        mz_zip_reader_file_stat(&zip, index, &file_stat);
        uv.resize((size_t)file_stat.m_uncomp_size / sizeof(float));
        mz_zip_reader_extract_file_to_mem(&zip, "UV.0", (void*)uv.data(), (size_t)file_stat.m_uncomp_size, 0);

        index = mz_zip_reader_locate_file(&zip, "BoneIndices4", "", 0);
        if(mz_zip_reader_file_stat(&zip, index, &file_stat) == MZ_TRUE) {
            boneIndices4.resize((size_t)file_stat.m_uncomp_size / sizeof(int32_t));
            mz_zip_reader_extract_file_to_mem(&zip, "BoneIndices4", (void*)boneIndices4.data(), (size_t)file_stat.m_uncomp_size, 0);
        }

        index = mz_zip_reader_locate_file(&zip, "BoneWeights4", "", 0);
        if(mz_zip_reader_file_stat(&zip, index, &file_stat) == MZ_TRUE) {
            boneWeights4.resize((size_t)file_stat.m_uncomp_size / sizeof(float));
            mz_zip_reader_extract_file_to_mem(&zip, "BoneWeights4", (void*)boneWeights4.data(), (size_t)file_stat.m_uncomp_size, 0);
        }

        SetAttribArray<Au::Position>(vertices);
        SetAttribArray<Au::Normal>(normals);
        SetAttribArray<Au::UV>(uv);
        SetAttribArray<Au::BoneIndex4>(boneIndices4);
        SetAttribArray<Au::BoneWeight4>(boneWeights4);
        SetIndices(indices);

        mz_zip_reader_end(&zip);
        return true;
    }

    virtual bool Serialize(std::vector<unsigned char>& data) {
        mz_zip_archive zip;
        memset(&zip, 0, sizeof(zip));
        mz_zip_writer_init_heap(&zip, 0, 0);

        int32_t vertexCount = this->vertexCount;
        int32_t indexCount = (int32_t)indices.size();
        mz_zip_writer_add_mem(&zip, "VertexCount", (void*)&vertexCount, sizeof(vertexCount), 0);
        mz_zip_writer_add_mem(&zip, "IndexCount", (void*)&indexCount, sizeof(indexCount), 0);
        mz_zip_writer_add_mem(&zip, "Indices", (void*)indices.data(), indexCount * sizeof(uint32_t), 0);
        // TODO: GET RID OF AU:: ATTRIBUTE IDENTIFIERS
        mz_zip_writer_add_mem(&zip, "Vertices", (void*)GetAttribBytes<Au::Position>().data(), vertexCount * 3 * sizeof(float), 0);
        mz_zip_writer_add_mem(&zip, MKSTR("Normals." << 0).c_str(), (void*)GetAttribBytes<Au::Normal>().data(), vertexCount * 3 * sizeof(float), 0);
        mz_zip_writer_add_mem(&zip, MKSTR("UV." << 0).c_str(), (void*)GetAttribBytes<Au::UV>().data(), vertexCount * 2 * sizeof(float), 0);
        
        if(!GetAttribBytes<Au::BoneIndex4>().empty() && !GetAttribBytes<Au::BoneWeight4>().empty()) {
            mz_zip_writer_add_mem(&zip, "BoneIndices4", (void*)GetAttribBytes<Au::BoneIndex4>().data(), vertexCount * 4 * sizeof(int32_t), 0);
            mz_zip_writer_add_mem(&zip, "BoneWeights4", (void*)GetAttribBytes<Au::BoneWeight4>().data(), vertexCount * 4 * sizeof(float), 0);
        }

        void* bufptr;
        size_t sz;
        mz_zip_writer_finalize_heap_archive(&zip, &bufptr, &sz);
        data = std::vector<unsigned char>((unsigned char*)bufptr, (unsigned char*)bufptr + sz);
        mz_zip_writer_end(&zip);
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

    vaoDirty = true;
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
