#ifndef EDITOR_ANIMATION_TIMELINE_H
#define EDITOR_ANIMATION_TIMELINE_H

#include <scene_object.h>
#include <util/imgui_wrapper.h>
#include <util/imgui_console.h>
#include <resources/resource/resource_ref.h>

#include "editor_window.h"

class EditorAnimationTimeline : public EditorWindow {
public:
    virtual void Draw() {
      if(ImGui::Begin("Timeline", &visible)) {
        if (ImGui::BeginCombo("Current anim", "Run2.anim", 0)) {
          

          ImGui::EndCombo();
        }
        ImGui::Separator();
        if(ImGui::BeginChild("test", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 300), false, 0)) {
          ImGui::Text("Test");
          ImGui::EndChild();
        }

        static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
        int new_count = 7;
        ImGui::End();
      }
    }
};

#endif
