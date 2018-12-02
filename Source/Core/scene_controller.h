#ifndef SCENE_CONTROLLER_H
#define SCENE_CONTROLLER_H

#include <util/frame_graph.h>
#include <rttr/type>
#include "component.h"

#include "scene_renderer.h"
#include "scene_physics.h"

#include <set>
#include "components/updatable.h"

class SceneObject;
class SceneController
{
public:
    SceneController();

    bool Init();

    SceneRenderer& GetRenderer() { return renderer; }
    ScenePhysics& GetPhysics() { return physics; }

    void SetScene(SceneObject* sceneObject);

    void Update();

    void _onAddComponent(rttr::type type, Component* c, SceneObject* so);
    void _onRemoveComponent(rttr::type type, Component* c, SceneObject* so);
private:
    void _firstTimeSceneProcess(SceneObject* s);
    SceneObject* scene;
    SceneRenderer renderer;
    ScenePhysics physics;
    std::set<Updatable*> updatables;
};

#endif
