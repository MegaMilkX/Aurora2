#ifndef GUI_ELEMENT_H
#define GUI_ELEMENT_H

#include <util/gfxm.h>
#include <component.h>

class GuiElement : public Component
{
    RTTR_ENABLE(Component)
public:
    gfxm::vec2 dimensions;
    gfxm::vec2 center;
    gfxm::vec4 margin;
    gfxm::vec4 padding;
    int layer;
};

#endif
