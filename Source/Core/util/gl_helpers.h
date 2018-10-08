#ifndef GL_HELPERS_H
#define GL_HELPERS_H

#include <aurora/gfx.h>

inline size_t glTypeSize(GLenum t)
{
    size_t sz;
    switch(t)
    {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE: sz = 1; break;
    case GL_SHORT:
    case GL_UNSIGNED_SHORT: sz = 2; break;
    case GL_INT:
    case GL_UNSIGNED_INT: sz = 4; break;
    case GL_HALF_FLOAT: sz = 2; break;
    case GL_FLOAT: sz = 4; break;
    case GL_DOUBLE: sz = 8; break;
    case GL_FIXED: sz = 4; break;
    case GL_INT_2_10_10_10_REV: sz = 4; break;
    case GL_UNSIGNED_INT_2_10_10_10_REV: sz = 4; break;
    case GL_UNSIGNED_INT_10F_11F_11F_REV: sz = 4; break;
    }
    return sz;
}

inline GLuint _initShader(const std::string& source, GLuint shaderProgram, GLuint type)
{
    const char* csource = source.c_str();
    GLuint vs = glCreateShader(type);
    glShaderSource(vs, 1, &csource, 0);
    glCompileShader(vs);

    GLint Result = GL_FALSE;
    int InfoLogLength;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 1)
    {
        std::vector<char> ShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(vs, InfoLogLength, NULL, &ShaderErrorMessage[0]);
        std::cout << &ShaderErrorMessage[0] << std::endl;
    }

    glAttachShader(shaderProgram, vs);
    return vs;
}

struct GLVertexBufferDesc
{
    std::string name;
    GLint size;
    GLenum type;
    GLboolean normalized;
    GLsizei stride;
    GLenum hint;
};

class GLVertexArrayObject
{
public:
    void Init(const std::vector<GLVertexBufferDesc>& buffersDesc)
    {
        this->buffersDesc = buffersDesc;
        
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        for(unsigned i = 0; i < buffersDesc.size(); ++i)
        {
            GLuint buf;
            glGenBuffers(1, &buf);
            glBindBuffer(GL_ARRAY_BUFFER, buf);
            buffers.push_back(buf);
        }
        glGenBuffers(1, &indexBuf);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
    }
    void Cleanup()
    {
        buffersDesc.clear();
        for(GLuint buf : buffers)
        {
            glDeleteBuffers(1, &buf);
        }
        buffers.clear();
        glDeleteVertexArrays(1, &vao);
    }
    void FillArrayBuffer(const std::string& name, const std::vector<unsigned char>& data)
    {
        FillArrayBuffer(name, (void*)data.data(), data.size());
    }
    void FillArrayBuffer(const std::string& name, void* data, size_t sz)
    {
        GLVertexBufferDesc desc;
        int bufId = _getBuf(name, desc);
        if(bufId == -1)
            return;
        glBindBuffer(GL_ARRAY_BUFFER, buffers[bufId]);
        glBufferData(GL_ARRAY_BUFFER, sz, data, desc.hint);
        glEnableVertexAttribArray(bufId);
        glVertexAttribPointer(bufId, desc.size, desc.type, desc.normalized, desc.stride, 0);
/*
        std::cout << name << std::endl;
        if(desc.type == GL_FLOAT) {
            size_t count = sz / 4 / desc.size;
            for(size_t i = 0; i < count; ++i) {
                float* fdata = (float*)data;
                for(size_t j = 0; j < desc.size; ++j) {
                    std::cout << " " << fdata[i * desc.size + j];
                }
                std::cout << std::endl;
            }
        }*/
    }
    void FillIndexBuffer(const std::vector<uint32_t>& data)
    {
        size_t szData = data.size() * sizeof(uint32_t);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, szData, (void*)data.data(), GL_STATIC_DRAW);
        indexCount = szData / sizeof(uint32_t);
    }
    void Bind() const 
    {
        glBindVertexArray(vao);
    }
    void DrawElements(GLenum primitiveType) const
    {
        glDrawElements(primitiveType, indexCount, GL_UNSIGNED_INT, (void*)0);
    }
    
    GLuint GetGlName() { return vao; } const
    std::vector<GLVertexBufferDesc>& Desc() const { return buffersDesc; }
private:
    int _getBuf(const std::string& name, GLVertexBufferDesc& descOut)
    {
        int i = 0;
        for(GLVertexBufferDesc& desc : buffersDesc)
        {
            if(desc.name == name)
            {
                descOut = desc;
                return i;
            }
            ++i;
        }
        return -1;
    }
    int indexCount;
    GLuint vao;
    std::vector<GLVertexBufferDesc> buffersDesc;
    std::vector<GLuint> buffers;
    GLuint indexBuf;
};

#endif
