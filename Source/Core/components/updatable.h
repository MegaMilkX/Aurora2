#ifndef UPDATABLE_H
#define UPDATABLE_H

#include "../scene_object.h"

class Updatable;
class UpdatableController : public SceneObject::Component
{
    RTTR_ENABLE(SceneObject::Component)
public:
    UpdatableController(){}

    void Add(Updatable* u);
    void Remove(Updatable* u);
    void Update();
private:
    std::vector<Updatable*> list;
};

class Updatable : public SceneObject::Component
{
    RTTR_ENABLE(SceneObject::Component)
friend UpdatableController;
public:
    ~Updatable()
    {
        GetObject()->Root()->Get<UpdatableController>()->Remove(this);
    }
    virtual void OnInit()
    {
        GetObject()->Root()->Get<UpdatableController>()->Add(this);
        OnStart();
    }

    virtual void OnStart() {}
    virtual void OnUpdate() = 0;
    virtual void OnCleanup() {}
};

inline void UpdatableController::Add(Updatable* u)
{
    list.emplace_back(u);
}

inline void UpdatableController::Remove(Updatable* u)
{
    for(unsigned i = 0; i < list.size(); ++i)
    {
        if(list[i] == u)
        {
            list.erase(list.begin() + i);
            break;
        }
    }
}

inline void UpdatableController::Update()
{
    for(auto u : list) u->OnUpdate();
}

#endif
