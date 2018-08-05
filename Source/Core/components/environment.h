#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <scene_object.h>
#include "../util/gfxm.h"

class Environment : public SceneObject::Component
{
public:
    gfxm::vec3 ambientColor;
    gfxm::vec3 rimLightColor;

    void AmbientColor(float r, float g, float b)
    { 
        ambientColor = gfxm::vec3(r, g, b);
        glClearColor (r, g, b, 1.0f);        
    }
    void RimColor(float r, float g, float b)
    { 
        rimLightColor = gfxm::vec3(r, g, b); 
    }
};

#endif
