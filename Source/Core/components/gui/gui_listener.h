#ifndef GUI_LISTENER_H
#define GUI_LISTENER_H

#include <component.h>
#include "gui_root.h"

class GuiListener : public Component
{
    RTTR_ENABLE(Component)
public:
    void OnInit()
    {
        Object()->Root()->Get<GuiRoot>();
    }
};

#endif
