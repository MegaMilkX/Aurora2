#ifndef EDITOR_DATA_PICK_H
#define EDITOR_DATA_PICK_H

#include <scene_object.h>
#include <util/imgui_wrapper.h>
#include <util/imgui_console.h>
#include <resources/resource/resource_ref.h>

class EditorWindow {
public:
    virtual ~EditorWindow() {}

    void Show() {
        visible = true;
    }
    void Update() {
        if(!visible) return;
        Draw();
    }

    virtual void Draw() = 0;
protected:
    bool visible = false;
};

class EditorDataPick : public EditorWindow {
public:
    EditorDataPick()
    : prop(rttr::type::get<void>().get_property("")) {

    }
    void SetTarget(rttr::property prop, SceneObject::Component* comp) { 
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
    SceneObject::Component* comp = 0;
    std::string selected_name;
};

#endif
