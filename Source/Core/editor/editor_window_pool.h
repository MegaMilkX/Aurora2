#ifndef EDITOR_WINDOW_POOL_H
#define EDITOR_WINDOW_POOL_H

#include <vector>
#include <memory>
#include "editor_window.h"

namespace Editor {
namespace GUI {

class WindowPool {
public:
    template<typename T>
    static std::shared_ptr<T> Create() {
        std::shared_ptr<T> ptr(new T());
        ptr->Show();
        windows.emplace_back(std::dynamic_pointer_cast<EditorWindow>(ptr));
        return ptr;
    }

    static void Update() {
        for(size_t i = 0; i < windows.size();) {
            if(!windows[i]->Update()) {
                windows.erase(windows.begin() + i);
            } else {
                ++i;
            }
        }
    }
private:
    static std::vector<std::shared_ptr<EditorWindow>> windows;
};

} // GUI
} // Editor

#endif
