#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <scene_object.h>
#include "../util/gfxm.h"

class Environment : public SceneObject::Component
{
    CLONEABLE(Environment)
    RTTR_ENABLE(SceneObject::Component)
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
STATIC_RUN(Environment)
{
    rttr::registration::class_<Environment>("Environment")
        .constructor<>()(rttr::policy::ctor::as_raw_ptr);
}

#endif
