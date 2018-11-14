#ifndef COMPONENT_LIGHT_OMNI_H
#define COMPONENT_LIGHT_OMNI_H

#include "../component.h"

#include <util/gfxm.h>

class LightDirect : public Component
{
    CLONEABLE(LightDirect)
    RTTR_ENABLE(Component)
public:
    LightDirect()
    : color(1.0f, 1.0f, 1.0f), intensity(1.0f) {}
    ~LightDirect()
    {
    }
    
    void Color(float r, float g, float b)
    { color = gfxm::vec3(r, g, b); }
    void Color(gfxm::vec3 col)
    { color = col; }
    gfxm::vec3 Color()
    { return color; }
    void Intensity(float i) {
        intensity = i;
    }
    float Intensity() {
        return intensity;
    }
    
    void OnInit()
    {
    }

    virtual bool _write(std::ostream& out, ExportData& exportData) {
        out.write((char*)&color, sizeof(color));
        out.write((char*)&intensity, sizeof(intensity));
        return true;
    }
    virtual bool _read(std::istream& in, size_t sz, ImportData& importData) {
        if(sz != sizeof(color) + sizeof(intensity)) 
            return false;
        in.read((char*)&color, sizeof(color));
        in.read((char*)&intensity, sizeof(intensity));
        return true;
    }
    virtual bool _editor() {
        static bool alpha_preview = true;
        static bool alpha_half_preview = false;
        static bool options_menu = true;
        static bool hdr = false;
        int misc_flags = (hdr ? ImGuiColorEditFlags_HDR : 0) | (alpha_half_preview ? ImGuiColorEditFlags_AlphaPreviewHalf : (alpha_preview ? ImGuiColorEditFlags_AlphaPreview : 0)) | (options_menu ? 0 : ImGuiColorEditFlags_NoOptions);
        ImGui::ColorEdit3("Color", (float*)&color, misc_flags);
        if(ImGui::DragFloat("Intensity", &intensity, intensity * 0.001f)) {}
        return true;
    }
private:
    gfxm::vec3 color;
    float intensity;
};
STATIC_RUN(LightDirect)
{
    rttr::registration::class_<LightDirect>("LightDirect")
        .constructor<>()(rttr::policy::ctor::as_raw_ptr)
        .property(
            "color",
            rttr::select_overload<gfxm::vec3(void)>(&LightDirect::Color),
            rttr::select_overload<void(gfxm::vec3)>(&LightDirect::Color)
        );
}

class LightOmni : public Component
{
    CLONEABLE(LightOmni)
    RTTR_ENABLE(Component)
public:
    LightOmni()
    : color(1.0f, 1.0f, 1.0f), intensity(1.0f) {}
    ~LightOmni()
    {
    }

    void Color(float r, float g, float b)
    {
        color = gfxm::vec3(r, g, b);
    }
    void Color(gfxm::vec3 col)
    {
        color = col;
    }
    void Intensity(float i)
    {
        intensity = i;
    }
    float Intensity()
    {
        return intensity;
    }
    
    gfxm::vec3 Color()
    { return color; }

    void OnInit()
    {
    }

    virtual bool _write(std::ostream& out, ExportData& exportData) {
        out.write((char*)&color, sizeof(color));
        out.write((char*)&intensity, sizeof(intensity));
        return true;
    }
    virtual bool _read(std::istream& in, size_t sz, ImportData& importData) {
        if(sz != sizeof(color) + sizeof(intensity)) 
            return false;
        in.read((char*)&color, sizeof(color));
        in.read((char*)&intensity, sizeof(intensity));
        return true;
    }
    virtual bool _editor() {
        static bool alpha_preview = true;
        static bool alpha_half_preview = false;
        static bool options_menu = true;
        static bool hdr = false;
        int misc_flags = (hdr ? ImGuiColorEditFlags_HDR : 0) | (alpha_half_preview ? ImGuiColorEditFlags_AlphaPreviewHalf : (alpha_preview ? ImGuiColorEditFlags_AlphaPreview : 0)) | (options_menu ? 0 : ImGuiColorEditFlags_NoOptions);
        ImGui::ColorEdit3("Color", (float*)&color, misc_flags);
        if(ImGui::DragFloat("Intensity", &intensity, intensity * 0.001f)) {}
        return true;
    }
private:
    gfxm::vec3 color;
    float intensity;
};
STATIC_RUN(LightOmni)
{
    rttr::registration::class_<LightOmni>("LightOmni")
        .constructor<>()(rttr::policy::ctor::as_raw_ptr)
        .property(
            "color", 
            rttr::select_overload<gfxm::vec3(void)>(&LightOmni::Color),
            rttr::select_overload<void(gfxm::vec3)>(&LightOmni::Color)
        )
        .property(
            "intensity", 
            rttr::select_overload<float(void)>(&LightOmni::Intensity),
            rttr::select_overload<void(float)>(&LightOmni::Intensity)
        );
}

#endif
