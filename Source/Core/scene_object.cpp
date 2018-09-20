#include "scene_object.h"
#include "scene_controller.h"

SceneObject::~SceneObject()
{
    //for(unsigned i = 0; i < objects.size(); ++i)
    //    delete objects[i];
    for(auto& kv : components)
    {
        if(controller)
        {
            controller->_onRemoveComponent(kv.first, kv.second, this);
        }
        delete kv.second;
    }
}

void SceneObject::AddComponent(Component* c, rttr::type t)
{
    c->object = this;
    components.insert(std::make_pair(t, c));
    c->OnInit();
    if(controller)
    {
        controller->_onAddComponent(t, c, this);
    }
}