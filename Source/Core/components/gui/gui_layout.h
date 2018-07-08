#ifndef GUI_LAYOUT_H
#define GUI_LAYOUT_H

#include <scene_object.h>
#include <overlay/quad.h>

#include <functional>

class GuiLayout : public SceneObject::Component
{
public:
    enum DOCKING
    {
        FILL = 0,
        UP = 1,
        DOWN = 2,
        LEFT = 4,
        RIGHT = 8
    };

    GuiLayout()
    : docking(FILL), parent(0) {}
    ~GuiLayout()
    {
        Detach();
    }

    std::function<void(const GuiLayout*)> onChange;

    DOCKING docking;
    gfxm::ivec2 size;
    gfxm::ivec4 margin;
    gfxm::ivec4 padding;
    gfxm::ivec4 bbox;

    gfxm::ivec4 Rebuild(const gfxm::ivec4& box)
    {
        gfxm::ivec4 b = box;
        gfxm::ivec4 bCut;
        switch(docking)
        {
        case FILL:
            bbox = b;
            bCut = { 0, 0, 0, 0 };
            break;
        case UP:
            bbox = { b.x, b.y, b.z, b.y + size.y };
            bCut = { b.x, b.y + size.y, b.z, b.w };
            break;
        case DOWN:
            bbox = { b.x, b.w - size.y, b.z, b.w };
            bCut = { b.x, b.y, b.z, b.w - size.y };
            break;
        case LEFT:
            bbox = { b.x, b.y, b.x + size.x, b.w };
            bCut = { b.x + size.x, b.y, b.z, b.w };
            break;
        case RIGHT:
            bbox = { b.z - size.x, b.y, b.z, b.w };
            bCut = { b.x, b.y, b.z - size.x, b.w };
            break;
        }
        Get<Transform>()->Position((float)bbox.x, (float)bbox.y, 0.0f);
        
        gfxm::ivec4 b2 = bbox;
        for(GuiLayout* l : children)
        {
            b2 = l->Rebuild(b2);
        }

        if(onChange) onChange(this);

        return bCut;
    }

    void Rebuild()
    {
        Rebuild(bbox);
    }

    void SetSize(int w, int h)
    {
        size = { w, h };
        RebuildParent();
    }

    void Dock(DOCKING dock)
    {
        docking = dock;
        RebuildParent();
    }

    void RebuildParent()
    {
        if(parent)
        {
            parent->Rebuild();
        }
    }

    void Attach(GuiLayout* l)
    {
        l->Detach();
        children.push_back(l);
        l->parent = this;
        Rebuild(bbox);
    }

    void OnInit()
    {
        size = { 100, 100 };
    }
private:
    void Detach()
    {
        if(parent)
        {
            for(unsigned i = 0; i < parent->children.size(); ++i)
            {
                if(parent->children[i] == this)
                {
                    parent->children.erase(parent->children.begin() + i);
                }
            }
        }
    }

    GuiLayout* parent;
    std::vector<GuiLayout*> children;
};

#endif
