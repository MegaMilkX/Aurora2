#ifndef AU_GFX_MESH_H
#define AU_GFX_MESH_H

#include "gl/glbuffer.h"

#include "../util/attribute.h"

namespace Au{
namespace GFX{

class Mesh
{
public:
    Mesh();
    ~Mesh();
    
    enum PRIMITIVE
    {
        POINT,
        LINE,
        TRIANGLE
    };
    
    struct SubMesh
    {
    friend Mesh;
        SubMesh(const std::string& name, Mesh* mesh, unsigned int indexCount, unsigned int offset)
        : name(name), mesh(mesh), indexCount(indexCount), offset(offset)
        {}

        void Bind()
        {
            mesh->Bind();
        }
        void Render()
        {
            mesh->Render(indexCount, offset);
        }
        
        void SetIndexCount(unsigned int value) { indexCount = value; }
        void SetOffset(unsigned int value) { offset = value; }
        
    private:
        std::string name;
        Mesh* mesh;
        unsigned int indexCount;
        unsigned int offset;
    };
    
    void PrimitiveType(PRIMITIVE prim);
    void Format(const std::vector<AttribInfo>& vertexFormat);
    std::vector<AttribInfo>& Format();
    template<typename VERTEX>
    void VertexData(const std::vector<VERTEX>& data);
    void VertexData(void* data, unsigned int count);
    template<typename ATTRIB, typename T>
    void VertexAttrib(const std::vector<T>& attribData);
    void VertexAttribByInfo(const AttribInfo& attrInfo, const std::vector<unsigned char>& data);
    void VertexAttribByInfo(const AttribInfo& attrInfo, const unsigned char* data, size_t sz);
    void IndexData(const std::vector<unsigned short>& data);
    void IndexData(void* data, unsigned int count);
    
    unsigned int IndexCount() { return indexCount; }
    
    void SetSubMesh(const std::string& name, unsigned int indexCount, unsigned int offset)
    {
        SubMesh* sm = GetSubMesh(name);
        sm->SetIndexCount(indexCount);
        sm->SetOffset(offset);
    }
    SubMesh* FindSubMesh(const std::string& name)
    {
        for(unsigned i = 0; i < subMeshes.size(); ++i)
            if(subMeshes[i].name == name)
                return &subMeshes[i];
        return 0;
    }
    SubMesh* GetSubMesh(const std::string& name)
    {
        SubMesh* sm = FindSubMesh(name);
        if(sm) return sm;
        subMeshes.push_back(SubMesh(name, this, 0, indexCount));
        return &(subMeshes[subMeshes.size() - 1]);
    }
    SubMesh* GetSubMesh(unsigned int i) { return &subMeshes[i]; }
    unsigned int SubMeshCount() { return subMeshes.size(); }
    
    void Bind();
    
    void Render(unsigned int indexCount, unsigned int offset = 0);
    
    void PrintFormat();
private:
    std::vector<AttribInfo> vertexFormat;
    size_t vertexSize;
    unsigned int vao;
    GLBuffer vertexBuffer;
    GLBuffer indexBuffer;
    unsigned int indexCount;
    
    GLenum primitive;
    
    std::vector<SubMesh> subMeshes;
    
    template<typename T, typename OrigT>
    void BlitAttribToBuffer(
        const std::vector<OrigT>& attribData,
        const std::vector<char>& dest,
        const AttribInfo& attrInfo
    );
};

template<typename VERTEX>
void Mesh::VertexData(const std::vector<VERTEX>& data)
{
    VertexData((void*)data.data(), data.size());
}

template<typename ATTRIB, typename T>
void Mesh::VertexAttrib(const std::vector<T>& attribData)
{
    if(vertexSize == 0)
        return;
    
    AttribInfo attrInfo = ATTRIB();
    
    unsigned newVertexCount = attribData.size() / attrInfo.elemCount;
    unsigned oldVertexCount = 0;
    
    std::vector<char> newBuffer(newVertexCount * vertexSize);
    
    if(vertexBuffer.Valid())
    {
        std::vector<char> oldData = vertexBuffer.Data();
        oldVertexCount = oldData.size() / vertexSize;
        if(oldVertexCount > newVertexCount)
            newBuffer.resize(oldVertexCount * vertexSize);
        for(unsigned i = 0; i < oldData.size(); ++i)
            newBuffer[i] = oldData[i];
    }
    
    if(attrInfo.elemType == TypeInfo<char>::Index())
        BlitAttribToBuffer<char>(attribData, newBuffer, attrInfo);
    else if(attrInfo.elemType == TypeInfo<unsigned char>::Index())
        BlitAttribToBuffer<unsigned char>(attribData, newBuffer, attrInfo);
    else if(attrInfo.elemType == TypeInfo<short>::Index())
        BlitAttribToBuffer<short>(attribData, newBuffer, attrInfo);
    else if(attrInfo.elemType == TypeInfo<unsigned short>::Index())
        BlitAttribToBuffer<unsigned short>(attribData, newBuffer, attrInfo);
    else if(attrInfo.elemType == TypeInfo<int>::Index())
        BlitAttribToBuffer<int>(attribData, newBuffer, attrInfo);
    else if(attrInfo.elemType == TypeInfo<unsigned int>::Index())
        BlitAttribToBuffer<unsigned int>(attribData, newBuffer, attrInfo);
    else if(attrInfo.elemType == TypeInfo<float>::Index())
        BlitAttribToBuffer<float>(attribData, newBuffer, attrInfo);
    else if(attrInfo.elemType == TypeInfo<double>::Index())
        BlitAttribToBuffer<double>(attribData, newBuffer, attrInfo);
    
    VertexData((void*)newBuffer.data(), newBuffer.size() / vertexSize);
}

template<typename T, typename OrigT>
void Mesh::BlitAttribToBuffer(
    const std::vector<OrigT>& attribData,
    const std::vector<char>& dest,
    const AttribInfo& attrInfo
)
{
    unsigned attrOffset = 0;
    for(unsigned i = 0; i < vertexFormat.size(); ++i)
    {
        if(vertexFormat[i] == attrInfo)
            break;
        else
            attrOffset += vertexFormat[i].elemCount * vertexFormat[i].elemSize;
    }
    
    if(attrOffset >= vertexSize)
        return;
    
    unsigned vertexCount = attribData.size() / attrInfo.elemCount;
    for(unsigned i = 0; i < vertexCount; ++i)
    {
        char* cursor = (char*)(dest.data() + i * vertexSize);
        cursor += attrOffset;
        for(unsigned j = 0; j < attrInfo.elemCount; ++j)
            *(((T*)cursor) + j) = (T)attribData[i * attrInfo.elemCount + j];
    }
}

}
}

#endif
