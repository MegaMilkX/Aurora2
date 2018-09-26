#ifndef QUAD_H
#define QUAD_H

#include <component.h>
#include "../../util/gfx_quad.h"

#include <mutex>

class OverlayRoot;
class Quad : public Component
{
    CLONEABLE(Quad)
    RTTR_ENABLE(Component)
public:
    GfxQuad quad;

    void SetSize(float width, float height)
    {
        quad.width = width;
        quad.height = height;
    }

    void SetColor(float r, float g, float b, float a)
    {
        quad.color = { r, g, b, a };
    }

    void SetImage(const std::string& name)
    {
        quad.image.set(name);
    }

    void OnInit()
    {
        Object()->Root()->GetComponent<OverlayRoot>();

        static std::once_flag once_flag;
        std::call_once(
            once_flag,
            [this](){
                resource<Texture2D> tex = 
                    resource<Texture2D>::get("$white_pixel");
                unsigned char color[4] = { 255, 255, 255, 255 };
                tex->Data(color, 1, 1, 4);
            }
        );
        quad.image = resource<Texture2D>::get("$white_pixel");
    }
};

#endif
