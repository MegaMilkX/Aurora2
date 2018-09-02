#ifndef EDITOR_COMPONENT_CREATOR_H
#define EDITOR_COMPONENT_CREATOR_H

#include <scene_object.h>
#include <util/imgui_wrapper.h>
#include <util/imgui_console.h>

class EditorComponentCreator {
public:
    void Show() {
        visible = true;
    }
    void Update(SceneObject* selectedObject) {
        if(!visible) return;
        
        if(ImGui::Begin("Component selector", &visible)) {
            rttr::array_range<rttr::type> list = rttr::type::get<SceneObject::Component>().get_derived_classes();
            for(auto& t : list) {
                bool selected = selectedCompType == t.get_name().to_string();
                if(ImGui::Selectable(t.get_name().to_string().c_str(), &selected)) {
                    selectedCompType = t.get_name().to_string();
                }
            }                

            if(ImGui::Button("Create")) {
                if(selectedObject) {
                    selectedObject->Get(selectedCompType);
                    visible = false;
                }
            }
            
            ImGui::End();
        }
    }
private:
    std::string selectedCompType = "";
    bool visible = false;
};

#endif
