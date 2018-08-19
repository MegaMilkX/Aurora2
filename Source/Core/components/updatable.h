#ifndef UPDATABLE_H
#define UPDATABLE_H

#include "../scene_object.h"

class Updatable : public SceneObject::Component
{
    RTTR_ENABLE(SceneObject::Component)
public:
    ~Updatable()
    {
    }
    virtual void OnInit()
    {
        OnStart();
    }

    virtual void OnStart() {}
    virtual void OnUpdate() = 0;
    virtual void OnCleanup() {}
};

#endif
