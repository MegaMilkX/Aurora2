#include <game_state.h>
#include <scene_object.h>

#include <transform.h>
#include <camera.h>
#include <model.h>
#include <light_omni.h>
#include <luascript.h>
#include <animator.h>
#include <skeleton.h>
#include <dynamics/rigid_body.h>
#include <collision/collider.h>
#include <sound_emitter.h>
#include <text_mesh.h>
#include <asset.h>

void Aurora2Init();

int main()
{
    init_resources();

    Meta::Print();

    GameState::Init();
    Aurora2Init();
    while(GameState::Update())
    {}
    GameState::Cleanup();
    
    return 0;
}