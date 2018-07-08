#include "gl_render_state.h"

namespace gl{

bool Shader::Init(GLenum type)
{
    id = glCreateShader(type);
    return true;
}
void Shader::Source(const std::string& src)
{
    const char* csrc = src.c_str();
    glShaderSource(id, 1, &csrc, 0);
}
bool Shader::Compile()
{
    glCompileShader(id);
    GLint res = GL_FALSE;
    int infoLogLen;
    glGetShaderiv(id, GL_COMPILE_STATUS, &res);
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLen);
    if(infoLogLen > 1)
    {
        std::vector<char> errMsg(infoLogLen + 1);
        glGetShaderInfoLog(id, infoLogLen, NULL, &errMsg[0]);
        std::cout << &errMsg[0] << std::endl;
    }
    if(res == GL_FALSE)
        return false;
    return true;
}
void Shader::Attach(GLuint program)
{
    glAttachShader(program, id);
}


ShaderProgram::ShaderProgram()
{
    id = glCreateProgram();
}
void ShaderProgram::AttachShader(Shader* shader)
{
    shader->Attach(id);
}
void ShaderProgram::BindAttrib(GLuint loc, const std::string& name)
{
    glBindAttribLocation(id, loc, name.c_str());
}
void ShaderProgram::BindFragData(GLuint loc, const std::string& name)
{
    glBindFragDataLocation(id, loc, name.c_str());
}
bool ShaderProgram::Link()
{
    glLinkProgram(id);
    int res;
    int infoLogLen;
    glValidateProgram(id);
    glGetProgramiv(id, GL_VALIDATE_STATUS, &res);
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLogLen);
    if(infoLogLen > 1)
    {
        std::vector<char> errMsg(infoLogLen + 1);
        glGetProgramInfoLog(id, infoLogLen, NULL, &errMsg[0]);
        std::cout << &errMsg[0] << std::endl;
    }
    if(res == GL_FALSE)
        return false;
    return true;
}
GLuint ShaderProgram::GetUniform(const std::string& name)
{
    return glGetUniformLocation(id, name.c_str());
}
void ShaderProgram::Use()
{
    glUseProgram(id);
}
GLuint ShaderProgram::GetId()
{
    return id;
}


void RenderState::SetShaderProgram(ShaderProgram* prog)
{
    this->prog = prog;
}
ShaderProgram* RenderState::GetShaderProgram()
{
    return prog;
}

}