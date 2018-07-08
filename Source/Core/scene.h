#ifndef SCENE_ROOT_H
#define SCENE_ROOT_H

#include <set>

#include "scene_object.h"

class ComponentUpdatable : public SceneObject::Component
{
public:
    virtual void Update()
    {

    }
};

class Scene : public SceneObject
{
public:
    void Update()
    {
        for(auto u : _updatables)
        {
            u->Update();
        }
    }

    void _addUpdatable(ComponentUpdatable* u)
    {
        _updatables.insert(u);
    }
    void _removeUpdatable(ComponentUpdatable* u)
    {
        _updatables.erase(u);
    }
private:
    std::set<ComponentUpdatable*> _updatables;
};

#endif
