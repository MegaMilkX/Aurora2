#ifndef COMPONENT_H
#define COMPONENT_H

#include "scene_object.h"
#include <util/imgui_wrapper.h>
#include <util/imgui_console.h>
#include <serialization/import_data.h>
#include <serialization/export_data.h>

class Component
{
    RTTR_ENABLE()
    friend SceneObject;
public:
    Component()
    : type(rttr::type::get<void>())
    {}
    virtual ~Component() {}

    virtual Component* clone() { return 0; }

    SceneObject* Object() { return object; }
    SceneObject* GetObject() { return object; }
    template<typename T>
    T* GetComponent()
    { return GetObject()->GetComponent<T>(); }
    template<typename T>
    T* Get() { return GetObject()->GetComponent<T>(); }
    template<typename T>
    T* RootGet() { return Object()->Root()->GetComponent<T>(); }

    SceneObject* CreateObject() { return GetObject()->CreateObject(); }
    
    rttr::type GetType() const { return type; }

    // Use to signal the SceneController (if attached) that this component has changed
    void Refresh() {
        if(object) {
            object->RefreshComponent(this);
        }
    }

    // Called after the component has been created
    virtual void OnInit() {}
    
    virtual bool _write(std::ostream& out, ExportData& exportData) {
        return false;
    }
    virtual bool _read(std::istream& in, size_t sz, ImportData& importData) {
        return false;
    }
    virtual bool _editor() {
        return false;
    }
protected:
    rttr::type type;
private:
    SceneObject* object;
};

#endif
