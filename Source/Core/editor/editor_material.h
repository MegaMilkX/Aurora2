#ifndef EDITOR_MATERIAL_H
#define EDITOR_MATERIAL_H

#include "editor_window.h"
#include <util/editor_gui_helpers.h>
#include <resources/material.h>

class EditorMaterial : public EditorWindow {
public:
    EditorMaterial() {

    }

    void SetTarget(std::shared_ptr<Material> tgt) {
        material = tgt;
    }

    virtual bool Draw() {
        if(ImGui::Begin("Material editor", &visible)) {
            if(material) {
                ButtonResource("Diffuse map", material->diffuseMap, ".png");
                ButtonResource("Normal map", material->normalMap, ".png");
                ButtonResource("Specular map", material->specularMap, ".png");
                static bool alpha_preview = true;
                static bool alpha_half_preview = false;
                static bool options_menu = true;
                static bool hdr = false;
                int misc_flags = (hdr ? ImGuiColorEditFlags_HDR : 0) | (alpha_half_preview ? ImGuiColorEditFlags_AlphaPreviewHalf : (alpha_preview ? ImGuiColorEditFlags_AlphaPreview : 0)) | (options_menu ? 0 : ImGuiColorEditFlags_NoOptions);
                ImGui::ColorEdit3("Tint", (float*)&material->tint, misc_flags);
                float gloss_step = 0.01f;
                if(ImGui::DragFloat("Glossiness", &material->glossiness, gloss_step, 0.0f, 1.0f)) {

                }
                if(ImGui::DragFloat("Emission", &material->emission, 0.01f, 0.0f, 1.0f)) {

                }
            } else {
                ImGui::Text("No material selected");
            }
            
            ImGui::End();
        }
        return true;
    }
private:
    std::shared_ptr<Material> material;
};

#endif
