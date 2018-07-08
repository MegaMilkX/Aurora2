#ifndef COMPONENT_GFXSCENE_H
#define COMPONENT_GFXSCENE_H

#include <vector>

#include <aurora/gfx.h>
#include <util/gfxm.h>
#include <aurora/transform.h>
#include "../scene_object.h"
#include <component.h>

#include "../lib/task_graph.h"

#undef GetObject

struct FrameCommon
{
    gfxm::mat4 projection;
    gfxm::mat4 view;
    gfxm::vec3 viewPos;
    float width, height;
    SceneObject* scene;
};

class Camera;
class Renderer : public SceneObject::Component
{
public:
    Renderer();
    
    task_graph::graph& GetFrameGraph() { return frameGraph; }
    
    void Render();
    void Render(const gfxm::mat4& projection,
        const gfxm::mat4& transform);
    
    void CurrentCamera(Camera* cam) { _currentCamera = cam; }
    Camera* CurrentCamera() { return _currentCamera; }
    
    void AmbientColor(float r, float g, float b)
    { 
        ambientColor = gfxm::vec3(r, g, b);
        glClearColor (r, g, b, 1.0f);        
    }
    void RimColor(float r, float g, float b)
    { rimColor = gfxm::vec3(r, g, b); }
        
    virtual void OnInit();
private:
    task_graph::graph frameGraph;
    
    Camera* _currentCamera;
    
    gfxm::vec3 ambientColor;
    gfxm::vec3 rimColor;
};
COMPONENT(Renderer)

#endif
