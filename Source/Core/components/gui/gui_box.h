#ifndef GUI_BOX_H
#define GUI_BOX_H

#include "gui_root.h"
#include "gui_layout.h"
#include "../overlay/quad.h"

class GuiBox : public SceneObject::Component
{
public:
    void OnResize(const GuiLayout* layout)
    {
        gfxm::ivec4 bbox = layout->bbox;

        Get<Quad>()->quad.color = { 0.2f, 0.2f, 0.2f, 1.0f };
        Get<Quad>()->quad.width = (float)(bbox.z - bbox.x);
        Get<Quad>()->quad.height = (float)(bbox.w - bbox.y);
        Get<Transform>()->Position((float)bbox.x, (float)bbox.y, 0.0f);
    }

    void OnInit()
    {
        Object()->Root()->Get<GuiRoot>();
        Get<GuiLayout>()->onChange = std::bind(&GuiBox::OnResize, this, std::placeholders::_1);
        SceneObject* parent = Object()->Parent();
        if(parent)
        {
            GuiLayout* p = parent->FindComponent<GuiLayout>();
            p->Attach(Get<GuiLayout>());
        }
    }
};

#endif
