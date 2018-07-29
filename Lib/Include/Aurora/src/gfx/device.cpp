#include "device.h"

#include <iostream>

namespace Au{
namespace GFX{

bool Device::Init(Window& window)
{
    PIXELFORMATDESCRIPTOR pfd = { 0 };
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 32;
    pfd.iLayerType = PFD_MAIN_PLANE;
    
    deviceContext = GetDC(window);
    
    int pixelFormat = ChoosePixelFormat(deviceContext, &pfd);
    
    if(!pixelFormat)
        return false;
    
    if(!SetPixelFormat(deviceContext, pixelFormat, &pfd))
        return false;
    
    HGLRC renderingContext = wglCreateContext(deviceContext);
    wglMakeCurrent(deviceContext, renderingContext);
    
    bool core_profile = false;
    WGLEXTLoadFunctions();
    if(!wglCreateContextAttribsARB)
    {
        context = renderingContext;
        threadingContext = wglCreateContext(deviceContext);
        wglMakeCurrent(NULL, NULL);
        wglShareLists(threadingContext, context);
        wglMakeCurrent(deviceContext, context);
    }
    else
    {
        int attr[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 5,
            WGL_CONTEXT_FLAGS_ARB, 0,
            0
        };
        
        HGLRC renderingContext3plus = wglCreateContextAttribsARB(deviceContext, 0, attr);
        
        if(!renderingContext3plus)
        {
            context = renderingContext;
            threadingContext = wglCreateContext(deviceContext);
            wglMakeCurrent(NULL, NULL);
            wglShareLists(threadingContext, context);
            wglMakeCurrent(deviceContext, context);
        }
        else
        {
            context = renderingContext3plus;
            threadingContext = wglCreateContextAttribsARB(deviceContext, context, attr);
            wglMakeCurrent(NULL,NULL);
            wglDeleteContext(renderingContext);
            wglMakeCurrent(deviceContext, context);
            core_profile = true;
        }
    }
    
    GLEXTLoadFunctions();

    if (core_profile)
    {
        //For newer context at least one vao is required
        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
    }
    
    int version[2];
    glGetIntegerv(GL_MAJOR_VERSION, &version[0]);
    glGetIntegerv(GL_MINOR_VERSION, &version[1]);
    contextVersion = version[0] * 100 + version[1] * 10;
    
    std::cout << "OpenGL v" << APIVersion() << " ready.\n";
    
    std::cout << "GLSL v" << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
    
    //=======================================================
    glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    wglSwapIntervalEXT(0);
    
    return true;
}

void Device::Cleanup()
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(context);
}

void Device::Clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Device::Render()
{
    boundState->Bind();
    boundSubMesh->Render();
}

void Device::SwapBuffers()
{
    ::SwapBuffers(deviceContext);
}

Mesh* Device::CreateMesh()
{
    wglMakeCurrent(deviceContext, context);
    Mesh* mesh = new Mesh();
    return mesh;
}

Texture2D* Device::CreateTexture2D()
{
    wglMakeCurrent(deviceContext, context);
    Texture2D* texture = new Texture2D();
    return texture;
}

Shader* Device::CreateShader(Shader::STAGE stage)
{
    wglMakeCurrent(deviceContext, context);
    Shader* shader = new Shader(stage);
    return shader;
}

RenderState* Device::CreateRenderState()
{
    wglMakeCurrent(deviceContext, context);
    RenderState* state = new RenderState();
    return state;
}

void Device::Destroy(Mesh* mesh)
{
    wglMakeCurrent(deviceContext, context);
    delete mesh;
}

void Device::Destroy(Texture2D* texture)
{
    wglMakeCurrent(deviceContext, context);
    delete texture;
}

void Device::Destroy(Shader* shader)
{
    wglMakeCurrent(deviceContext, context);
    delete shader;
}

void Device::Bind(Mesh* mesh)
{
    Bind(mesh->GetSubMesh(0));
}

void Device::Bind(Mesh::SubMesh* subMesh)
{
    boundSubMesh = subMesh;
    boundSubMesh->Bind();
}

void Device::Bind(RenderState* state)
{
    boundState = state;
}

int Device::APIVersion()
{
    return contextVersion;
}

}
}