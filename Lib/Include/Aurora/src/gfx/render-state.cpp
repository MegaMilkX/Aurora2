#include "render-state.h"

namespace Au{
namespace GFX{

RenderState::RenderState()
: shaderProgram(0),
depthTest(true)
{
    
}

RenderState::~RenderState()
{
    if(shaderProgram)
        glDeleteProgram(shaderProgram);
}

void RenderState::AttribFormat(const std::vector<AttribInfo>& vertexFormat)
{
    _createProgramIfNotExists();
    
    //std::cout << "Formatting shader attribs" << std::endl;
    this->vertexFormat = vertexFormat;
    std::map<typeindex, AttribInstance> instanceCounters;
    for(unsigned int i = 0; i < vertexFormat.size(); ++i)
    {
        const AttribInfo& attrInfo = vertexFormat[i];
        unsigned int instance = instanceCounters[attrInfo.typeIndex]++;
        AttribIndex globalAttribIndex = 
            GetGlobalAttribIndex(attrInfo.typeIndex, instance);
        
        std::string shaderInputName = attrInfo.name;
        if(instance > 0)
            shaderInputName += std::to_string(instance);
                                 
        glBindAttribLocation(shaderProgram, globalAttribIndex, shaderInputName.c_str());
        //attrInfo.Print();
        //std::cout << "globalAttribIndex: " << globalAttribIndex << std::endl;
    }
    
    _linkProgram();
}
void RenderState::SetShader(Shader* shaderStage)
{
    shaders.insert(std::make_pair(shaderStage->Stage(), shaderStage->_uniqueIndex()));
    if(shaderStage->Stage() == Au::GFX::Shader::VERTEX)
        _deductAttribFormat(shaderStage->Source());
    _linkProgram();
}

void RenderState::AddSampler2D(const std::string& name, int layer)
{
    samplerLayers[layer] = name;
}

int RenderState::GetSampler2DLayer(const std::string& name)
{
    std::map<int, std::string>::iterator it;
    for(it = samplerLayers.begin(); it != samplerLayers.end(); ++it)
    {
        if(it->second == name)
            return it->first;
    }
    
    return 0;
}

void RenderState::DepthTest(bool flag)
{
    depthTest = flag;
}

void RenderState::Bind()
{
    if(depthTest)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
    
    glUseProgram(shaderProgram);
    for(unsigned int i = 0; i < uniforms.size(); ++i)
    {
        uniforms[i].Upload(uniformLocations[i]);
    }
}

void RenderState::_createProgramIfNotExists()
{
    if(shaderProgram > 0)
        return;
    shaderProgram = glCreateProgram();
}

void RenderState::_linkProgram()
{
    _createProgramIfNotExists();
    
    if(shaders.size() < 2)
        return;
    
    std::map<Shader::STAGE, unsigned int>::iterator it = shaders.begin();
    for(it; it != shaders.end(); ++it)
    {
        glAttachShader(shaderProgram, it->second);
    }
    
    glLinkProgram(shaderProgram);
    
    GLint Result = GL_FALSE;
    int InfoLogLength;

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &Result);
    glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &InfoLogLength);
    
    if (InfoLogLength > 0)
    {
        std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
        glGetProgramInfoLog(shaderProgram, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        statusString += &ProgramErrorMessage[0];
    }
    
    it = shaders.begin();
    for(it; it != shaders.end(); ++it)
    {
        glDetachShader(shaderProgram, it->second);
    }
    
    Bind();
    std::map<int, std::string>::iterator itSampler;
    for(itSampler = samplerLayers.begin(); itSampler != samplerLayers.end(); ++itSampler)
    {
        glUniform1i(glGetUniformLocation(shaderProgram, itSampler->second.c_str()), itSampler->first);
    }
}

void RenderState::_deductAttribFormat(const std::string& source)
{
    Au::GLSLStitch::Snippet snip =
        Au::GLSLStitch::MakeSnippet(source);
    Au::AttribFormat attribFormat;
    std::vector<Au::AttribInfo> attribs = _getAttribList();
            
    for(unsigned i = 0; i < snip.inputs.size(); ++i)
    {
        Au::GLSLStitch::Variable& var =
            snip.inputs[i];
    
        for(unsigned j = 0; j < attribs.size(); ++j)
        {
            Au::AttribInfo& attr = attribs[j];
            int r = _tryMatchStr(var.name, attr.name);
            if(r)
            {
                attribFormat << attr;
                break;
            }
        }
    }
    //attribFormat.Print();
    AttribFormat(attribFormat);
}

std::vector<Au::AttribInfo>& RenderState::_getAttribList()
{
    static std::vector<Au::AttribInfo> attribs =
        _getAttribListInit();
    return attribs;
}

bool RenderState::_compareAttribByNameLen(const Au::AttribInfo& first, const Au::AttribInfo& second)
{ return first.name.size() > second.name.size(); }

std::vector<Au::AttribInfo> RenderState::_getAttribListInit()
{
    std::vector<Au::AttribInfo> result;
    
    result.push_back(Au::Position());
    result.push_back(Au::Normal());
    result.push_back(Au::Tangent());
    result.push_back(Au::Bitangent());
    result.push_back(Au::UV());
    result.push_back(Au::UVW());
    result.push_back(Au::ColorRGBA());
    result.push_back(Au::ColorRGB());
    result.push_back(Au::BoneWeight4());
    result.push_back(Au::BoneIndex4());
    
    std::sort(result.begin(), result.end(), &_compareAttribByNameLen);
    
    return result;
}

int RenderState::_tryMatchStr(const std::string& str, std::string& token)
{
    if(str.size() < token.size())
        return 0;
    
    for(unsigned i = 0; i < token.size(); ++i)
    {
        if(token[i] != str[i])
            return 0;
    }

    return token.size();
}

}
}
