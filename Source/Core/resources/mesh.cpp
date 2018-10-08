#include "mesh.h"

void Mesh::SetIndices(const std::vector<unsigned>& data)
{ 
    indices = data;
    vaoDirty = true;
}

std::vector<unsigned char>& Mesh::GetAttribBytesByName(const std::string& name)
{
    for(auto& kv : attribArrays)
    {
        if(kv.first.name == name)
            return kv.second;
    }
    return attribArrays[Au::Empty()];
}

std::vector<uint32_t>& Mesh::GetIndices()
{
    return indices;
}

GLuint Mesh::GetVao()
{
    if(vaoDirty) {
        std::vector<GLAttribDesc> vertexDesc = {
            { "Position", 3, GL_FLOAT, GL_FALSE },
            { "UV", 2, GL_FLOAT, GL_FALSE },
            { "Normal", 3, GL_FLOAT, GL_FALSE }
        };
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
        vertexArrayObject.Init(desc);
        for(const GLAttribDesc& d : vertexDesc) {
            vertexArrayObject.FillArrayBuffer(d.name, GetAttribBytesByName(d.name));
        }
        vertexArrayObject.FillIndexBuffer(GetIndices());
        vaoDirty = false;
        return vertexArrayObject.GetGlName();
    } else {
        return vertexArrayObject.GetGlName();
    }
}

GLuint Mesh::GetSkinVao()
{
    if(vaoSkinDirty) {
        std::vector<GLAttribDesc> vertexDesc = {
            { "Position", 3, GL_FLOAT, GL_FALSE },
            { "UV", 2, GL_FLOAT, GL_FALSE },
            { "Normal", 3, GL_FLOAT, GL_FALSE },
            { "BoneIndex4", 4, GL_FLOAT, GL_FALSE },
            { "BoneWeight4", 4, GL_FLOAT, GL_FALSE }
        };
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
        vertexArrayObjectSkin.Init(desc);
        for(const GLAttribDesc& d : vertexDesc) {
            vertexArrayObjectSkin.FillArrayBuffer(d.name, GetAttribBytesByName(d.name));
        }
        vertexArrayObjectSkin.FillIndexBuffer(GetIndices());
        vaoSkinDirty = false;
        return vertexArrayObjectSkin.GetGlName();
    } else {
        return vertexArrayObjectSkin.GetGlName();
    }
}

unsigned Mesh::GetIndexCount() 
{ return indices.size(); }