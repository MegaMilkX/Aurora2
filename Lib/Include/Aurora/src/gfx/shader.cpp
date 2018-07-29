#include "shader.h"

#include <vector>

namespace Au{
namespace GFX{

Shader::Shader(STAGE stage)
: stage(stage),
  glShader(0)
{
    
}

Shader::~Shader()
{
    if(glShader)
        glDeleteShader(glShader);
}
    
void Shader::Source(const std::string& source)
{
    unsigned int type;
    if(stage == VERTEX)
        type = GL_VERTEX_SHADER;
    else if (stage == PIXEL)
        type = GL_FRAGMENT_SHADER;
    else
        return;
    
    this->source = source;
    
    glShader = glCreateShader(type);
    
    const char* csource = source.c_str();
    
    glShaderSource(glShader, 1, &csource, 0);
    glCompileShader(glShader);
    
    GLint Result = GL_FALSE;
    int InfoLogLength;
    
    glGetShaderiv(glShader, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(glShader, GL_INFO_LOG_LENGTH, &InfoLogLength);
    
    if (InfoLogLength > 1)
    {
        std::vector<char> ShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(glShader, InfoLogLength, NULL, &ShaderErrorMessage[0]);
        statusString += &ShaderErrorMessage[0];
    }
}
    
}
}