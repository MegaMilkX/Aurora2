#ifndef COLLIDER_H
#define COLLIDER_H

#include <component.h>

class Collider : public Component {
    RTTR_ENABLE(Component)
public:


    virtual bool _write(std::ostream& out, ExportData& exportData) {
        //out.write((char*)&color, sizeof(color));
        //out.write((char*)&intensity, sizeof(intensity));
        return true;
    }
    virtual bool _read(std::istream& in, size_t sz, ImportData& importData) {
        //if(sz != sizeof(color) + sizeof(intensity)) 
        //    return false;
        //in.read((char*)&color, sizeof(color));
        //in.read((char*)&intensity, sizeof(intensity));
        return true;
    }
    virtual bool _editor() {
        /*
        static bool alpha_preview = true;
        static bool alpha_half_preview = false;
        static bool options_menu = true;
        static bool hdr = false;
        int misc_flags = (hdr ? ImGuiColorEditFlags_HDR : 0) | (alpha_half_preview ? ImGuiColorEditFlags_AlphaPreviewHalf : (alpha_preview ? ImGuiColorEditFlags_AlphaPreview : 0)) | (options_menu ? 0 : ImGuiColorEditFlags_NoOptions);
        ImGui::ColorEdit3("Color", (float*)&color, misc_flags);
        if(ImGui::DragFloat("Intensity", &intensity, intensity * 0.001f)) {}
        */
        if (ImGui::BeginCombo("Type", "Box", 0)) // The second parameter is the label previewed before opening the combo.
        {
            bool is_selected = false;
            if(ImGui::Selectable("Box", is_selected)) {}
            if(ImGui::Selectable("Sphere", is_selected)) {}
            if(ImGui::Selectable("Cyllinder", is_selected)) {}
            if(ImGui::Selectable("Capsule", is_selected)) {}
            if(ImGui::Selectable("Plane", is_selected)) {}
            if(ImGui::Selectable("Infinite plane", is_selected)) {}
            if(ImGui::Selectable("Concave mesh", is_selected)) {}
            if(ImGui::Selectable("Convex mesh", is_selected)) {}
            ImGui::EndCombo();
        }
        return true;
    }
};
STATIC_RUN(Collider)
{
    rttr::registration::class_<Collider>("Collider")
        .constructor<>()(rttr::policy::ctor::as_raw_ptr);
}

#endif
