#ifndef TEXTURE_3D_H
#define TEXTURE_3D_H

#include <aurora/gfx.h>
#include "../util/bitmap.h"

class Texture3D
{
public:
    Texture3D()
    : glTexName(0), dirty(true)
    {}
    ~Texture3D()
    {
        glDeleteTextures(1, &glTexName);
    }
    unsigned char* GetData()
    {
        return _data.data();
    }
    void Data(unsigned char* data, int width, int height, int depth, int bpp)
    {
        _data = std::vector<unsigned char>(data, data + bpp * width * height * depth);
        this->width = width;
        this->height = height;
        this->depth = depth;
        this->bpp = bpp;
        dirty = true;
    }
    void Blit2d(unsigned char* src, int w, int h, int bpp, int x, int y, int z)
    {
        unsigned char* dest = _data.data() + width * height * bpp * z;
        BmpBlit(dest, width, height, this->bpp, src, w, h, bpp, x, y);
        Data(_data.data(), width, height, depth, bpp);
    }
    GLuint GetGlName()
    {
        if(glTexName == 0)
            _initGlData();
        if(dirty)
            _reloadGlBuffer();
        return glTexName;
    }
private:
    bool dirty;
    std::vector<unsigned char> _data;
    int width, height, depth;
    int bpp;
    GLuint glTexName;

    void _initGlData()
    {
        glGenTextures(1, &glTexName);
        glBindTexture(GL_TEXTURE_3D, glTexName);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    void _reloadGlBuffer()
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        GLenum format;
        if(bpp == 1) format = GL_RED;
        else if(bpp == 2) format = GL_RG;
        else if(bpp == 3) format = GL_RGB;
        else if(bpp == 4) format = GL_RGBA;
        else return;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, glTexName);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, width, height, depth, 0, format, GL_UNSIGNED_BYTE, (const GLvoid*)_data.data());
        dirty = false;
    }
};

#endif
