#ifndef GBUFFER_H
#define GBUFFER_H

#include <aurora/gfx.h>

class GBuffer
{
public:
    GBuffer()
    : fbo(0), albedo(0), position(0), normal(0), specular(0)
    {}
    void Init(unsigned width, unsigned height)
    {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        ResizeBuffers(width, height);  
    }
    void Cleanup()
    {
        glDeleteFramebuffers(1, &fbo);
    }

    void ResizeBuffers(unsigned width, unsigned height)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        _createBuffer(albedo, GL_RGB, GL_UNSIGNED_BYTE, width, height, 0);
        _createBuffer(position, GL_RGB16F, GL_FLOAT, width, height, 1);
        _createBuffer(normal, GL_RGB16F, GL_FLOAT, width, height, 2);
        _createBuffer(specular, GL_RED, GL_UNSIGNED_BYTE, width, height, 3);

        if(depth) glDeleteTextures(1, &depth);
        glGenTextures(1, &depth);
        glBindTexture(GL_TEXTURE_2D, depth);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
            (GLsizei)width, (GLsizei)height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth, 0);

        GLenum a[4] = { 
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2,
            GL_COLOR_ATTACHMENT3
        };
        glDrawBuffers(4, a);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            LOG_ERR("G-Buffer is incomplete!");
        }
    }

    void Bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    }
    void Unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void _createBuffer(GLuint& t, GLint internalFormat, GLenum type, unsigned width, unsigned height, int attachment)
    {
        if(t) glDeleteTextures(1, &t);
        glGenTextures(1, &t);
        glBindTexture(GL_TEXTURE_2D, t);
        glTexImage2D(
            GL_TEXTURE_2D, 0, internalFormat, 
            (GLsizei)width, (GLsizei)height, 0, GL_RGB, type, 0
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, t, 0);
    }
    GLuint fbo;
    GLuint albedo;
    GLuint position;
    GLuint normal;
    GLuint specular;
    GLuint depth;
};

#endif
