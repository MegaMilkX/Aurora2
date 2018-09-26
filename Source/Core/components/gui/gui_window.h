#ifndef GUI_WINDOW_H
#define GUI_WINDOW_H

#include "gui_root.h"
#include "gui_layout.h"
#include "../overlay/quad.h"
#include "../overlay/text2d.h"
#include <functional>

class GuiWindow : public Component
{
    RTTR_ENABLE(Component)
public:
    void OnMouseEnter()
    {
        titleBar->Get<Quad>()->SetColor(0.7f, 0.7f, 0.7f, 1.0f);
    }
    void OnMouseLeave()
    {
        titleBar->Get<Quad>()->SetColor(0.4f, 0.4f, 0.4f, 1.0f);
    }
    void OnMouseMove(const eMouseMove* e)
    {
        if(drag)
        {
            Get<Transform>()->Position((float)(e->x - dragPoint.x), (float)(e->y - dragPoint.y), 0.0f);
        }
    }
    void OnMouseDown(const eMouseDown* e)
    {
        if(e->key == Au::Input::KEY_LBUTTON)
        {
            gfxm::vec3 pos = Get<Transform>()->WorldPosition();
            dragPoint = { e->x - (int)pos.x, e->y - (int)pos.y };
            drag = true;
        }
    }
    void OnMouseUp(const eMouseUp* e)
    {
        if(e->key == Au::Input::KEY_LBUTTON)
        {
            drag = false;
        }
    }
    void OnResize(const GuiLayout* layout)
    {
        gfxm::ivec4 bbox = layout->bbox;

        titleBar->Get<Quad>()->quad.height = 25;
        titleBar->Get<Quad>()->quad.width = (float)(bbox.z - bbox.x);
        clientArea->Get<Quad>()->quad.height = (float)(bbox.w - bbox.y - 25);
        clientArea->Get<Quad>()->quad.width = (float)(bbox.z - bbox.x);
    }

    gfxm::ivec2 dragPoint;
    bool drag = false;

    void OnInit()
    {
        Object()->Root()->Get<GuiRoot>();

        Get<GuiListener>()->onMouseEnter = std::bind(&GuiWindow::OnMouseEnter, this);
        Get<GuiListener>()->onMouseLeave = std::bind(&GuiWindow::OnMouseLeave, this);
        Get<GuiListener>()->onMouseMove = std::bind(&GuiWindow::OnMouseMove, this, std::placeholders::_1);
        Get<GuiListener>()->onMouseDown = std::bind(&GuiWindow::OnMouseDown, this, std::placeholders::_1);
        Get<GuiListener>()->onMouseUp = std::bind(&GuiWindow::OnMouseUp, this, std::placeholders::_1);

        titleBar = Object()->CreateObject();
        titleBar->Get<Quad>()->quad.height = 25;
        titleBar->Get<Quad>()->quad.width = 300;
        titleBar->Get<Quad>()->quad.color = { 0.4f, 0.4f, 0.4f, 1.0f };
        titleBar->Get<Transform>()->AttachTo(Get<Transform>());

        clientArea = Object()->CreateObject();
        clientArea->Get<Quad>()->quad.height = 500;
        clientArea->Get<Quad>()->quad.width = 300;
        clientArea->Get<Quad>()->quad.color = { 0.2f, 0.2f, 0.2f, 1.0f };
        clientArea->Get<Transform>()->AttachTo(Get<Transform>());
        clientArea->Get<Transform>()->Translate(0.0f, 25.0f, 0.0f);

        SceneObject* title = Object()->CreateObject();
        title->Get<Text2d>()->SetText("Window");
        title->Get<Text2d>()->SetSize(14);
        title->Get<Transform>()->AttachTo(Get<Transform>());
        title->Get<Transform>()->Translate(5.0f, 0.0f, 0.0f);

        Get<GuiLayout>()->onChange = std::bind(&GuiWindow::OnResize, this, std::placeholders::_1);
        SceneObject* parent = Object()->Parent();
        if(parent)
        {
            GuiLayout* p = parent->FindComponent<GuiLayout>();
            p->Attach(Get<GuiLayout>());
        }
    }

    SceneObject* titleBar;
    SceneObject* clientArea;
};

#endif
