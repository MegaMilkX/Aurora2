#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <component.h>
#include "../util/gfxm.h"

class Environment : public Component
{
    CLONEABLE(Environment)
    RTTR_ENABLE(Component)
public:
    gfxm::vec3 ambientColor = gfxm::vec3(0.5f, 0.5f, 0.5f);
    gfxm::vec3 rimLightColor;

    void AmbientColor(float r, float g, float b)
    { 
        ambientColor = gfxm::vec3(r, g, b);
        glClearColor (r, g, b, 1.0f);        
    }
    void RimColor(float r, float g, float b)
    { 
        rimLightColor = gfxm::vec3(r, g, b); 
    }

    virtual bool _write(std::ostream& out, ExportData& exportData) {
        out.write((char*)&ambientColor, sizeof(ambientColor));
        out.write((char*)&rimLightColor, sizeof(rimLightColor));
        return true;
    }
    virtual bool _read(std::istream& in, size_t sz, ImportData& importData) {
        if(sz != sizeof(ambientColor) + sizeof(rimLightColor)) 
            return false;
        in.read((char*)&ambientColor, sizeof(ambientColor));
        in.read((char*)&rimLightColor, sizeof(rimLightColor));
        return true;
    }
    virtual bool _editor() {
        static bool alpha_preview = true;
        static bool alpha_half_preview = false;
        static bool options_menu = true;
        static bool hdr = false;
        int misc_flags = (hdr ? ImGuiColorEditFlags_HDR : 0) | (alpha_half_preview ? ImGuiColorEditFlags_AlphaPreviewHalf : (alpha_preview ? ImGuiColorEditFlags_AlphaPreview : 0)) | (options_menu ? 0 : ImGuiColorEditFlags_NoOptions);
        ImGui::ColorEdit3("Ambient color", (float*)&ambientColor, misc_flags);
        ImGui::ColorEdit3("Rim color", (float*)&rimLightColor, misc_flags);
        return true;
    }
};
STATIC_RUN(Environment)
{
    rttr::registration::class_<Environment>("Environment")
        .constructor<>()(rttr::policy::ctor::as_raw_ptr)
        .property("AmbientColor", &Environment::ambientColor);
}

#endif
