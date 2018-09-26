#ifndef COMPONENT_LIGHT_OMNI_H
#define COMPONENT_LIGHT_OMNI_H

#include "../component.h"

#include <util/gfxm.h>

class LightDirect : public Component
{
    CLONEABLE(LightDirect)
    RTTR_ENABLE(Component)
public:
    LightDirect()
    : color(1.0f, 1.0f, 1.0f) {}
    ~LightDirect()
    {
    }
    
    void Color(float r, float g, float b)
    { color = gfxm::vec3(r, g, b); }
    void Color(gfxm::vec3 col)
    { color = col; }
    gfxm::vec3 Color()
    { return color; }
    
    void OnInit()
    {
    }
private:
    gfxm::vec3 color;
};
STATIC_RUN(LightDirect)
{
    rttr::registration::class_<LightDirect>("LightDirect")
        .constructor<>()(rttr::policy::ctor::as_raw_ptr)
        .property(
            "color",
            rttr::select_overload<gfxm::vec3(void)>(&LightDirect::Color),
            rttr::select_overload<void(gfxm::vec3)>(&LightDirect::Color)
        );
}

class LightOmni : public Component
{
    CLONEABLE(LightOmni)
    RTTR_ENABLE(Component)
public:
    LightOmni()
    : color(1.0f, 1.0f, 1.0f), intensity(1.0f) {}
    ~LightOmni()
    {
    }

    void Color(float r, float g, float b)
    {
        color = gfxm::vec3(r, g, b);
    }
    void Color(gfxm::vec3 col)
    {
        color = col;
    }
    void Intensity(float i)
    {
        intensity = i;
    }
    float Intensity()
    {
        return intensity;
    }
    
    gfxm::vec3 Color()
    { return color; }

    void OnInit()
    {
    }
private:
    gfxm::vec3 color;
    float intensity;
};
STATIC_RUN(LightOmni)
{
    rttr::registration::class_<LightOmni>("LightOmni")
        .constructor<>()(rttr::policy::ctor::as_raw_ptr)
        .property(
            "color", 
            rttr::select_overload<gfxm::vec3(void)>(&LightOmni::Color),
            rttr::select_overload<void(gfxm::vec3)>(&LightOmni::Color)
        )
        .property(
            "intensity", 
            rttr::select_overload<float(void)>(&LightOmni::Intensity),
            rttr::select_overload<void(float)>(&LightOmni::Intensity)
        );
}

#endif
