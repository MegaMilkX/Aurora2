#ifndef SCENE_CONTROLLER_H
#define SCENE_CONTROLLER_H

#include <util/frame_graph.h>
#include <rttr/type>
#include "scene_object.h"

#include "scene_renderer.h"

#include <set>
#include "components/updatable.h"

class SceneObject;
class SceneController
{
public:
    SceneController();

    bool Init();

    SceneRenderer& GetRenderer() { return renderer; }

    void SetScene(SceneObject* sceneObject);

    void Update();

    void _onAddComponent(rttr::type type, SceneObject::Component* c, SceneObject* so);
    void _onRemoveComponent(rttr::type type, SceneObject::Component* c, SceneObject* so);
private:
    void _firstTimeSceneProcess(SceneObject* s);
    SceneObject* scene;
    SceneRenderer renderer;
    std::set<Updatable*> updatables;
};

#endif
