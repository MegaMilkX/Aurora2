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

std::vector<unsigned>& Mesh::GetIndices()
{
    return indices;
}

GLuint Mesh::GetVao(/*const std::vector<GLAttribDesc>& vertexDesc*/)
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
/*
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
    
    return vao.GetGlName();*/
}

unsigned Mesh::GetIndexCount() 
{ return indices.size(); }