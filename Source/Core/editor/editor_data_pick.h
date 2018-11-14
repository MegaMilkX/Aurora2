#ifndef EDITOR_DATA_PICK_H
#define EDITOR_DATA_PICK_H

#include <component.h>
#include <util/imgui_wrapper.h>
#include <util/imgui_console.h>
#include <resources/resource/resource_ref.h>

#include "editor_window.h"

#include <util/has_suffix.h>

template<typename T>
class EditorDataPick : public EditorWindow {
public:
    EditorDataPick()
    : res_target(0) {

    }

    void SetSuffix(const std::string& suff) {
        suffix = suff;
    }
    void SetTarget(std::shared_ptr<T>* res) {
        res_target = res;
    }

    virtual bool Draw() {
        if(ImGui::Begin("Global data selector", &visible)) {
            for(size_t i = 0; i < GlobalDataRegistry().Count(); ++i) {
                std::string name = GlobalDataRegistry().GetNameById(i);
                if(!has_suffix(name, suffix)) {
                    continue;
                }
                bool selected = selected_name == name;
                if(ImGui::Selectable(name.c_str(), &selected)) {
                    if(name == selected_name) {
                        if(res_target) {
                            *res_target = GlobalResourceFactory().Get<T>(name);
                        }     
                    }
                    selected_name = name;
                }
            }
                  
            ImGui::End();
            return true;
        } else {
            return false;
        }
    }
private:
    std::string suffix;
    std::shared_ptr<T>* res_target;
    std::string selected_name;
};

#endif
