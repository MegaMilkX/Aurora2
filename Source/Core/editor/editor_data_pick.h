#ifndef EDITOR_DATA_PICK_H
#define EDITOR_DATA_PICK_H

#include <component.h>
#include <util/imgui_wrapper.h>
#include <util/imgui_console.h>
#include <resources/resource/resource_ref.h>

#include "editor_window.h"

class EditorDataPick : public EditorWindow {
public:
    EditorDataPick()
    : prop(rttr::type::get<void>().get_property("")) {

    }
    void SetTarget(rttr::property prop, Component* comp) { 
        this->prop = prop;
        this->comp = comp;
    }
    virtual void Draw() {
        if(!comp) return;

        if(ImGui::Begin("Global data selector", &visible)) {
            for(size_t i = 0; i < GlobalDataRegistry().Count(); ++i) {
                std::string name = GlobalDataRegistry().GetNameById(i);
                bool selected = selected_name == name;
                if(ImGui::Selectable(name.c_str(), &selected)) {
                    selected_name = name;
                }
            }

            if(ImGui::Button("Select")) {
                rttr::variant var = prop.get_value(comp);
                i_resource_ref& ref = var.get_value<i_resource_ref>();
                ref.set_from_factory(GlobalResourceFactory(), selected_name);
                prop.set_value(comp, var);
            }
            
            ImGui::End();
        }
    }
private:
    //i_resource_ref* tgt_resource_ref;
    rttr::property prop;
    Component* comp = 0;
    std::string selected_name;
};

#endif
