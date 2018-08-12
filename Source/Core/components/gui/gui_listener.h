#ifndef GUI_LISTENER_H
#define GUI_LISTENER_H

#include <scene_object.h>
#include "gui_root.h"

class GuiListener : public SceneObject::Component
{
    RTTR_ENABLE(SceneObject::Component)
public:
    void OnInit()
    {
        Object()->Root()->Get<GuiRoot>();
    }
};

#endif
