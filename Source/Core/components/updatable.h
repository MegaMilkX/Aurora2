#ifndef UPDATABLE_H
#define UPDATABLE_H

#include "../component.h"

class Updatable : public Component
{
    RTTR_ENABLE(Component)
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
