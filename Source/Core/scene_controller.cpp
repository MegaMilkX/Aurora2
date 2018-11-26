#include "scene_controller.h"

SceneController::SceneController()
    : scene(0) 
{}

bool SceneController::Init()
{
    renderer.Init();
    physics.Init();
    return true;
}

void SceneController::SetScene(SceneObject* sceneObject){
    // TODO: Cleanup
    scene = sceneObject;
    scene->SetController(this);
    // TODO: Hook up callbacks and stuff
    _firstTimeSceneProcess(sceneObject);
}

void SceneController::Update(){
    for(auto u : updatables) {
        u->OnUpdate();
    }

    physics.Update();
    physics.DebugDraw();
    renderer.Render();
}

void SceneController::_onAddComponent(
    rttr::type type, 
    Component* c, 
    SceneObject* so)
{
    if(type.is_derived_from<Updatable>()) {
        updatables.insert((Updatable*)c);
    }

    physics._onAddComponent(type, c, so);
    renderer._onAddComponent(type, c, so);
}

void SceneController::_onRemoveComponent(
    rttr::type type, 
    Component* c, 
    SceneObject* so)
{
    if(type.is_derived_from<Updatable>()) {
        updatables.erase((Updatable*)c);
    }

    physics._onRemoveComponent(type, c, so);
    renderer._onRemoveComponent(type, c, so);
}


void SceneController::_firstTimeSceneProcess(SceneObject* s) {
    for(unsigned i = 0; i < s->ChildCount(); ++i) {
        SceneObject* child = s->GetChild(i);
        _firstTimeSceneProcess(child);
    }
    for(unsigned i = 0; i < s->ComponentCount(); ++i) {
        Component* c = s->GetComponent(i);
        _onAddComponent(c->GetType(), c, s);
    }
}