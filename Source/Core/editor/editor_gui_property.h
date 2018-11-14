#ifndef EDITOR_GUI_PROPERTY_H
#define EDITOR_GUI_PROPERTY_H

#include <rttr/type>
#include <util/imgui_wrapper.h>
#include <util/imgui_console.h>
#include <functional>

namespace Editor {
namespace GUI {

class Property {
public:
    typedef std::function<void(void)> property_gui_func_t;

    static bool Draw(rttr::property prop, void* instance) {
        auto it = functions.find(prop.get_type());
        if(it == functions.end()) {
            ImGui::Text(("[unsupported]" + prop.get_name().to_string()).c_str());
        } else {
            it->second();
        }
        return true;
    }
    template<typename T>
    static void Register(property_gui_func_t func) {
        functions[rttr::type::get<T>()] = func;
    }

private:
    static std::map<rttr::type, property_gui_func_t> functions;
};

} // GUI
} // Editor

#endif
