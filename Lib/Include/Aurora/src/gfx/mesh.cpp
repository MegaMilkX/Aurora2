#include "mesh.h"

#include <map>

namespace Au{
namespace GFX{
    
std::map<typeindex, GLenum> MakeGLTypeMap()
{
    std::map<typeindex, GLenum> typemap;
    typemap[TypeInfo<char>::Index()] = GL_BYTE;
    typemap[TypeInfo<unsigned char>::Index()] = GL_UNSIGNED_BYTE;
    typemap[TypeInfo<short>::Index()] = GL_SHORT;
    typemap[TypeInfo<unsigned short>::Index()] = GL_UNSIGNED_SHORT;
    typemap[TypeInfo<int>::Index()] = GL_INT;
    typemap[TypeInfo<unsigned int>::Index()] = GL_UNSIGNED_INT;
    // GL_HALF_FLOAT
    typemap[TypeInfo<float>::Index()] = GL_FLOAT;
    typemap[TypeInfo<double>::Index()] = GL_DOUBLE;
    // GL_FIXED
    // GL_INT_2_10_10_10_REV
    // GL_UNSIGNED_INT_2_10_10_10_REV
    // GL_UNSIGNED_INT_10F_11F_11F_REV
    
    return typemap;
}
    
GLenum GetGLType(typeindex index)
{
    static std::map<typeindex, GLenum> typemap = MakeGLTypeMap();
    return typemap[index];
}

Mesh::Mesh()
: vertexSize(0)
{
    PrimitiveType(TRIANGLE);
    
    glGenVertexArrays(1, &vao);
    Bind();
    vertexBuffer = GLBuffer::Create(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
    vertexBuffer.Bind();
    indexBuffer = GLBuffer::Create(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
    indexBuffer.Bind();
    
    subMeshes.push_back(SubMesh("", this, 0, 0));
}

Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &vao);
    vertexBuffer.Destroy();
    indexBuffer.Destroy();
}

void Mesh::PrimitiveType(PRIMITIVE prim)
{
    if(prim == POINT)
        primitive = GL_POINTS;
    else if(prim == LINE)
        primitive = GL_LINES;
    else if(prim == TRIANGLE)
        primitive = GL_TRIANGLES;
}

void Mesh::Format(const std::vector<AttribInfo>& vertexFormat)
{
    this->vertexFormat = vertexFormat;
    vertexSize = 0;
    for(unsigned int i = 0; i < vertexFormat.size(); ++i)
    {
        vertexSize += vertexFormat[i].elemSize * vertexFormat[i].elemCount;
    }
    
    Bind();
    
    //std::cout << "Formatting mesh data" << std::endl;
    
    size_t offset = 0;
    std::map<typeindex, AttribInstance> instanceCounters;
    for(unsigned int i = 0; i < vertexFormat.size(); ++i)
    {
        const AttribInfo& attrInfo = vertexFormat[i];
        AttribIndex globalAttribIndex = 
            GetGlobalAttribIndex(attrInfo.typeIndex,
                                 instanceCounters[attrInfo.typeIndex]++);
        GLboolean normalized = GL_FALSE;
        if(GetGLType(attrInfo.elemType) == GL_UNSIGNED_BYTE)
            normalized = GL_TRUE;
        glVertexAttribPointer(globalAttribIndex,                    //*attrib index
                              attrInfo.elemCount,                   //*attrib element count
                              GetGLType(attrInfo.elemType),         //*element type
                              normalized,                           //*normalize?
                              vertexSize,                           //*stride
                              (void*)offset);                       //*offset
        offset += attrInfo.elemSize * attrInfo.elemCount;
        glEnableVertexAttribArray(globalAttribIndex);
        
        //attrInfo.Print();
        //std::cout << "globalAttribIndex: " << globalAttribIndex << std::endl;
    }
}

std::vector<AttribInfo>& Mesh::Format()
{
    return vertexFormat;
}

void Mesh::VertexData(void* data, unsigned int count)
{
    if(vertexSize == 0)
        return;
    
    vertexBuffer.Data(data, vertexSize * count);
}

void Mesh::VertexAttribByInfo(const AttribInfo& attrInfo, const std::vector<unsigned char>& data)
{
    VertexAttribByInfo(attrInfo, data.data(), data.size());
}
void Mesh::VertexAttribByInfo(const AttribInfo& attrInfo, const unsigned char* data, size_t sz)
{
    unsigned newVertexCount = sz / attrInfo.elemSize / attrInfo.elemCount;
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
    
#define BLIT_ATTRIB(ELEM_TYPE) \
        BlitAttribToBuffer<ELEM_TYPE>( \
            std::vector<ELEM_TYPE>( \
                (ELEM_TYPE*)data, \
                (ELEM_TYPE*)data + sz / sizeof(ELEM_TYPE) \
            ), \
            newBuffer, \
            attrInfo \
        )
    
    if(attrInfo.elemType == TypeInfo<char>::Index())
        BLIT_ATTRIB(char);
    else if(attrInfo.elemType == TypeInfo<unsigned char>::Index())
        BLIT_ATTRIB(unsigned char);
    else if(attrInfo.elemType == TypeInfo<short>::Index())
        BLIT_ATTRIB(short);
    else if(attrInfo.elemType == TypeInfo<unsigned short>::Index())
        BLIT_ATTRIB(unsigned short);
    else if(attrInfo.elemType == TypeInfo<int>::Index())
        BLIT_ATTRIB(int);
    else if(attrInfo.elemType == TypeInfo<unsigned int>::Index())
        BLIT_ATTRIB(unsigned int);
    else if(attrInfo.elemType == TypeInfo<float>::Index())
        BLIT_ATTRIB(float);
    else if(attrInfo.elemType == TypeInfo<double>::Index())
        BLIT_ATTRIB(double);
#undef BLIT_ATTRIB
    VertexData((void*)newBuffer.data(), newBuffer.size() / vertexSize);
}

void Mesh::IndexData(const std::vector<unsigned short>& data)
{
    IndexData((void*)data.data(), data.size());
}

void Mesh::IndexData(void* data, unsigned int count)
{
    if(count == 0)
        return;
    
    indexBuffer.Data(data, sizeof(unsigned short) * count);
    indexCount = count;
    //index_type = GL_UNSIGNED_SHORT;
    
    subMeshes[0].indexCount = indexCount;
}

void Mesh::Bind()
{
    glBindVertexArray(vao);
}

void Mesh::Render(unsigned int indexCount, unsigned int offset)
{
    glDrawElements(primitive, indexCount, GL_UNSIGNED_SHORT, (void*)offset);
}

void Mesh::PrintFormat()
{
    for(unsigned int i = 0; i < vertexFormat.size(); ++i)
    {
        vertexFormat[i].Print();
    }
}
 
}
}