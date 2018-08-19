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
}

#include <iostream>
void Renderer::Render()
{
    
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
    
    frameGraph.run();
}
