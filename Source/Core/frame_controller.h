#ifndef AAAAAAAAAAAA_H
#define AAAAAAAAAAAA_H

#include <component.h>
#include <map>
#include <vector>

class FrameController {
public:
    void FrameBegin() {
        // TODO:
    }
    void FrameUpdate() {
        // TODO:
    }
    void FrameEnd() {
        // TODO:
    }
private:
    std::map<rttr::type, Component*> new_components;
    std::map<rttr::type, Component*> removed_components;
};

#endif
