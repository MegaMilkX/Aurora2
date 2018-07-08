#include "renderer.h"

#include "model.h"
#include "skeleton.h"
#include "material.h"
#include "light_omni.h"
#include "camera.h"
#include "text_mesh.h"

Renderer::Renderer()
{
}

void Renderer::OnInit()
{
    GetObject()->GetComponent<Camera>();
}

#include <iostream>
void Renderer::Render()
{
    _currentCamera->Render();
}

void Renderer::Render(const gfxm::mat4& projection,
        const gfxm::mat4& transform)
{
    frameGraph.set_data(
        FrameCommon{
            projection,
            gfxm::inverse(transform),
            transform[3],
            (float)Common.frameSize.x, (float)Common.frameSize.y,
            GetObject() 
        }
    );
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, Common.frameSize.x, Common.frameSize.y);
    frameGraph.run();
}
