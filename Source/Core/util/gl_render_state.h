#ifndef RENDER_STATE_H
#define RENDER_STATE_H

#include <string>
#include <aurora/gfx.h>

namespace gl{

class Shader
{
public:
    bool Init(GLenum type);
    void Source(const std::string& src);
    bool Compile();
    void Attach(GLuint program);
private:
    GLuint id;
};

class ShaderProgram
{
public:
    ShaderProgram();
    void AttachShader(Shader* shader);
    void BindAttrib(GLuint loc, const std::string& name);
    void BindFragData(GLuint loc, const std::string& name);
    bool Link();
    GLuint GetUniform(const std::string& name);
    void Use();
    GLuint GetId();
private:
    GLuint id;
};

class RenderState
{
public:
    void SetShaderProgram(ShaderProgram* prog);
    ShaderProgram* GetShaderProgram();
private:
    ShaderProgram* prog;
};

}

#endif
