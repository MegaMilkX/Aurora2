#ifndef EDITOR_GUI_HELPERS_H
#define EDITOR_GUI_HELPERS_H

#include "dialog_write_data.h"
#include <editor/editor_window_pool.h>
#include <editor/editor_data_pick.h>

template<typename T>
inline void ButtonResource(const std::string& name, std::shared_ptr<T>& res, const std::string& suffix, typename EditorDataPick<T>::on_pick_fn_t cb = nullptr) {
    std::string button_text = name + ": (empty)";
    if(res) {
        if(res->Storage() == Resource::LOCAL) {
            button_text = name + ": (embedded)";
        } else if(res->Storage() == Resource::GLOBAL) {
            button_text = name + ": " + res->Name();
        }
    }
    if(ImGui::Button(button_text.c_str())) {
        auto window = Editor::GUI::WindowPool::Add(std::shared_ptr<EditorDataPick<T>>(new EditorDataPick<T>(cb)));
        window->SetSuffix(suffix);
        window->SetTarget(&res);
    }
    if(res) {
        ImGui::SameLine();
        if(ImGui::Button(("Export '" + suffix + "'...").c_str())) {
            DialogExportResource(res, suffix);
        }
    }
}

#endif
