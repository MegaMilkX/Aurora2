#ifndef GUI_ELEMENT_H
#define GUI_ELEMENT_H

#include <util/gfxm.h>
#include <scene_object.h>

class GuiElement : public SceneObject::Component
{
public:
    gfxm::vec2 dimensions;
    gfxm::vec2 center;
    gfxm::vec4 margin;
    gfxm::vec4 padding;
    int layer;
};

#endif
