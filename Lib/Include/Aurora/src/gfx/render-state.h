#ifndef RENDERSTATE_H
#define RENDERSTATE_H

#include <set>

#include "../util/attribute.h"
#include "../glslstitch/glslstitch.h"
#include "shader.h"
#include "uniform.h"

namespace Au{
namespace GFX{

class RenderState
{
public:
    RenderState();
    ~RenderState();
    void AttribFormat(const std::vector<AttribInfo>& vertexFormat);
    std::vector<AttribInfo> AttribFormat() { return vertexFormat; }
    
    void SetShader(Shader* shaderStage);
    template<typename T>
    void AddUniform(const std::string& name, unsigned int count = 1);
    void AddSampler2D(const std::string& name, int layer);
    int GetSampler2DLayer(const std::string& name);
    
    void DepthTest(bool);
    
    void Bind();
    
    std::string StatusString() { return statusString; }
private:
    GLuint shaderProgram;
    std::map<Shader::STAGE, unsigned int> shaders;
    std::vector<AttribInfo> vertexFormat;
    
    std::vector<unsigned int> uniformLocations;
    std::vector<Uniform> uniforms;
    
    std::map<int, std::string> samplerLayers;
    
    std::string statusString;
    
    bool depthTest;
    
    void _createProgramIfNotExists();
    void _linkProgram();
    void _deductAttribFormat(const std::string& source);
    std::vector<Au::AttribInfo>& _getAttribList();
    static bool _compareAttribByNameLen(const Au::AttribInfo& first, const Au::AttribInfo& second);
    std::vector<Au::AttribInfo> _getAttribListInit();
    int _tryMatchStr(const std::string& str, std::string& token);
};

template<typename T>
void RenderState::AddUniform(const std::string& name, unsigned int count)
{
    if(count == 0)
        return;
    
    Uniform uniform = GetUniform<T>(name, count);
    
    uniformLocations.push_back(glGetUniformLocation(shaderProgram, name.c_str()));
    uniforms.push_back(uniform);
}
   
}
}

#endif
