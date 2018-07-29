#ifndef AU_GFX_TEXTURE2D_H
#define AU_GFX_TEXTURE2D_H

#include "gl/glextutil.h"

namespace Au{
namespace GFX{
    
class Texture2D
{
public:
    Texture2D()
    {
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    ~Texture2D()
    {
        glDeleteTextures(1, &textureId);
    }
    
    void Data(unsigned char* data, int bpp, int width, int height)
    {
        GLenum format;
        if(bpp == 1) format = GL_RED;
        else if(bpp == 2) format = GL_RG;
        else if(bpp == 3) format = GL_RGB;
        else if(bpp == 4) format = GL_RGBA;
        else return;
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, (const GLvoid*)data);
    }
    
    void Bind(int layer = 0)
    {
        glActiveTexture(GL_TEXTURE0 + layer);
        glBindTexture(GL_TEXTURE_2D, textureId);
    }
private:
    GLuint textureId;
};
    
}
}

#endif
