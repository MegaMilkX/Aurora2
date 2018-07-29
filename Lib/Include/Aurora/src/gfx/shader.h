#ifndef SHADER_H
#define SHADER_H

#include <string>
#include "gl/glextutil.h"

namespace Au{
namespace GFX{

class Shader
{
public:
    enum STAGE
    {
        VERTEX,
        PIXEL
    };
    Shader(STAGE stage);
    ~Shader();
    
    void Source(const std::string& source);
    std::string& Source() { return source; }
    std::string StatusString(){ return statusString; }
    
    STAGE Stage() { return stage; }
    
    unsigned int _uniqueIndex() { return glShader; }
private:
    std::string source;
    STAGE stage;
    GLuint glShader;
    std::string statusString;
};
 
}
}

#endif
