#ifndef COMPONENT_H
#define COMPONENT_H

#include "scene_object.h"

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

    virtual void OnInit() {}
protected:
    rttr::type type;
private:
    SceneObject* object;
};

#endif
