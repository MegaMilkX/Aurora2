#ifndef GFXDEVICE_H
#define GFXDEVICE_H

#include "../window/window.h"
#include "mesh.h"
#include "texture2d.h"
#include "render-state.h"

namespace Au{
namespace GFX{

class Device
{
public:
    bool Init(Window& window);
    void Cleanup();
    
    void Clear();
    void Render();
    void SwapBuffers();
    
    void SetContextCurrent() { wglMakeCurrent(deviceContext, context); }
    
    Mesh* CreateMesh();
    Texture2D* CreateTexture2D();
    Shader* CreateShader(Shader::STAGE stage);
    RenderState* CreateRenderState();
    void Destroy(Mesh* mesh);
    void Destroy(Texture2D* texture);
    void Destroy(Shader* shader);
    
    void Bind(Mesh* mesh);
    void Bind(Mesh::SubMesh* subMesh);
    void Bind(RenderState* state);
    template<typename T>
    void Set(Uniform uniform, const T& data)
    {
        uniform = data;
    }
    
    RenderState* BoundState() { return boundState; }
    
    int APIVersion();
    
    // Render sequence operators
    template<typename T>
    Device& operator<<(T* data) { return *this; }
    template<typename T>
    Device& operator<<(const T& data)
    {
        currentUniform = data;
        return *this;
    }
private:
    HDC deviceContext;
    HGLRC context;
    HGLRC threadingContext;
    int contextVersion = 0;
    
    Mesh::SubMesh* boundSubMesh;
    RenderState* boundState;
    Uniform currentUniform;
};

template<>
inline Device& Device::operator<<(RenderState* state)
{
    Bind(state);
    return *this;
}
template<>
inline Device& Device::operator<<(Mesh* mesh)
{
    Bind(mesh);
    Render();
    return *this;
}

}
}

#endif
