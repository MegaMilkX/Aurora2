#ifndef TEXT_2D_H
#define TEXT_2D_H

#undef GetObject

#include <component.h>
#include <texture2d.h>
#include <mesh.h>

#include <font_data.h>
#include "../lib/nimbusmono_bold.otf.h"
#include "../../util/bitmap.h"
#include "../../util/gfx_text.h"

class OverlayRoot;
class Text2d : public Component
{
    CLONEABLE
    RTTR_ENABLE(Component)
public:
    Text2d()
    {
        text = new GfxText();
        text->SetSize(16);
    }
    ~Text2d()
    {
        delete text;
    }
    void SetText(const std::string& str)
    {
        SetText(std::vector<int>(str.begin(), str.end()));
    }
    void SetText(const std::vector<int>& str)
    {
        text->SetText(str);
        text->Rebuild();
    }

    void SetSize(unsigned sz)
    {
        text->SetSize(sz);
        text->Rebuild();
    }

    void SetFont(const std::string& name)
    {
        text->SetFont(name);
        text->Rebuild();
    }

    void OnInit()
    {
        GetObject()->Root()->GetComponent<OverlayRoot>();
    }
    GfxText* text;
};

#endif
